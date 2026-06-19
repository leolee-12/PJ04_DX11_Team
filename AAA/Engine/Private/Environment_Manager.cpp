#include "Environment_Manager.h"
#include "DirectTK/DDSTextureLoader.h"

CEnvironment_Manager::CEnvironment_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CEnvironment_Manager::Load_Cube(const _tchar* pPath, ID3D11ShaderResourceView** ppSRV, _uint* pMipCount)
{
    ID3D11Resource* pResource = { nullptr };
    if (FAILED(DirectX::CreateDDSTextureFromFile(m_pDevice, pPath, &pResource, ppSRV)))
        return E_FAIL;

    // 밉 수는 리소스에서 직접 읽음 (큐브맵마다 다를 수 있음)
    *pMipCount = 1;
    ID3D11Texture2D* pTex = { nullptr };
    if (SUCCEEDED(pResource->QueryInterface(IID_PPV_ARGS(&pTex))))
    {
        D3D11_TEXTURE2D_DESC td{};
        pTex->GetDesc(&td);
        *pMipCount = td.MipLevels;
        Safe_Release(pTex);
    }
    Safe_Release(pResource); // SRV가 리소스 ref를 유지하므로 해제 OK

    return S_OK;
}

HRESULT CEnvironment_Manager::Register(const _wstring& strTag, const _tchar* pDiffuseDDS, const _tchar* pSpecularDDS, const _tchar* pLUTDDS, _float fIntensity)
{
    if (m_Registry.find(strTag) != m_Registry.end())
        return S_OK; // 이미 등록됨

    ENVIRONMENT_DESC desc;
    desc.fIntensity = fIntensity;

    _uint iMipDiff = 1, iMipSpec = 1;
    if (FAILED(Load_Cube(pDiffuseDDS, &desc.pDiffuseSRV, &iMipDiff)))
        return E_FAIL;
    if (FAILED(Load_Cube(pSpecularDDS, &desc.pSpecularSRV, &iMipSpec)))
    {
        Safe_Release(desc.pDiffuseSRV);
        return E_FAIL;
    }
    desc.iSpecularMip = iMipSpec;

    if (pLUTDDS)   // LUT은 3D라 Load_Cube(밉카운트) 말고 직접 로드
    {
        ID3D11Resource* pRes = nullptr;
        if (FAILED(DirectX::CreateDDSTextureFromFile(m_pDevice, pLUTDDS, &pRes, &desc.pColorGradeLUT)))
            return E_FAIL;
        Safe_Release(pRes);
    }

    m_Registry.emplace(strTag, desc);

    // 첫 등록을 폴백/현재로
    if (nullptr == m_Default.pSpecularSRV)
        m_Default = desc;
    if (nullptr == m_Current.pSpecularSRV)
        m_Current = desc;

    return S_OK;
}

HRESULT CEnvironment_Manager::Set_Current(const _wstring& strTag)
{
    auto iter = m_Registry.find(strTag);
    if (iter == m_Registry.end())
    {
        m_Current = m_Default; // 폴백
        return E_FAIL;
    }
    m_Current = iter->second;
    return S_OK;
}

CEnvironment_Manager* CEnvironment_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CEnvironment_Manager(pDevice, pContext);
}

void CEnvironment_Manager::Free()
{
    __super::Free();

    // registry가 SRV 소유 → 여기서만 해제 (m_Current/m_Default는 차용 포인터라 중복해제 X)
    for (auto& Pair : m_Registry)
    {
        Safe_Release(Pair.second.pDiffuseSRV);
        Safe_Release(Pair.second.pSpecularSRV);
        Safe_Release(Pair.second.pColorGradeLUT);
    }
    m_Registry.clear();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}