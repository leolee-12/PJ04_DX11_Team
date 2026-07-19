#include "Light_Manager.h"
#include "Light.h"

CLight_Manager::CLight_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

const LIGHT_DESC* CLight_Manager::Get_LightDesc(_uint iIndex)
{
    if (iIndex >= m_Lights.size())
        return nullptr;

    auto    iter = m_Lights.begin();

    for (size_t i = 0; i < iIndex; i++)
        ++iter;

    if (iter == m_Lights.end())
        return nullptr;
    
    return (*iter)->Get_LightDesc();    
}

_int CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
    CLight* pLight = CLight::Create(m_pDevice, m_pContext, LightDesc);
    if (nullptr == pLight)
        return -1;

    m_Lights.push_back(pLight);
    return static_cast<_int>(m_Lights.size()) - 1;
}

HRESULT CLight_Manager::Set_LightDesc(_uint iIndex, const LIGHT_DESC& LightDesc)
{
    if (iIndex >= m_Lights.size())
        return E_FAIL;

    auto iter = m_Lights.begin();
    std::advance(iter, iIndex);
    (*iter)->Set_LightDesc(LightDesc);
    return S_OK;
}

HRESULT CLight_Manager::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
    for (auto& pLight : m_Lights)
    {
        if (LIGHT::DIRECTIONAL == pLight->Get_LightDesc()->eType)
            continue;   // Á÷»ç±¤Àº ¼ÎÀÌ´õ ±Û·Î¹ú·Î ÀÌ°üµÊ

        if (FAILED(pLight->Render(pShader, pVIBuffer)))
            return E_FAIL;
    }
    return S_OK;

    return S_OK;
}

HRESULT CLight_Manager::Clear_Lights()
{
    for (auto& pLight : m_Lights)
        Safe_Release(pLight);

    m_Lights.clear();

	return S_OK;
}

CLight_Manager* CLight_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CLight_Manager(pDevice, pContext);
}

void CLight_Manager::Free()
{
    __super::Free();

    for (auto& pLight : m_Lights)
        Safe_Release(pLight);

    m_Lights.clear();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);


}
