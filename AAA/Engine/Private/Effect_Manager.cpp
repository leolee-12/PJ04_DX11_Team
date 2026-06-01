#include "Effect_Manager.h"
#include "GameInstance.h"
#include "GameObject.h"

CEffect_Manager::CEffect_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
    , m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CEffect_Manager::Initialize()
{
    m_p2DShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Effect_VtxTex.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
    if (m_p2DShader == nullptr)
        return E_FAIL;

    return S_OK;
}

CEffect_Manager* CEffect_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Manager* pInstance = new CEffect_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CEffect_Manager");
        Safe_Release(pInstance);
        return nullptr;
    }
    return pInstance;
}

void CEffect_Manager::Free()
{
    Safe_Release(m_p2DShader);
    Safe_Release(m_pMeshShader);

    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}



HRESULT CEffect_Manager::Spawn(_uint iLevel,
    const _wstring& strProtoTag,
    const CEffect::EFFECT_DESC& desc,
    CEffect** ppOut)
{
    // 같은 프로토타입을 빠르게 여러 번 발화해도 이름 충돌 안 나게 카운터로 고유화
    _wstring strObjTag = strProtoTag + L"#" + std::to_wstring(m_iSpawnCounter++);

    CGameObject* pObj = nullptr;
    HRESULT hr = m_pGameInstance_Proxy->Add_GameObject_Return(
        &pObj,
        iLevel, strProtoTag,
        iLevel, EFFECT_LAYER_TAG, strObjTag,
        const_cast<CEffect::EFFECT_DESC*>(&desc));

    if (FAILED(hr) || nullptr == pObj)
        return E_FAIL;

    CEffect* pEffect = dynamic_cast<CEffect*>(pObj);
    if (nullptr == pEffect)
    {
        // 프로토타입이 CEffect 파생이 아니면 즉시 회수
        m_pGameInstance_Proxy->Destroy_GameObject(pObj);
        return E_FAIL;
    }

    if (ppOut)
        *ppOut = pEffect;

    return S_OK;
}