#include "GorillaNamePlate.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Model.h"

CGorillaNamePlate::CGorillaNamePlate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBossNamePlate(pDevice, pContext) {
}
CGorillaNamePlate::CGorillaNamePlate(const CGorillaNamePlate& Prototype)
    : CBossNamePlate(Prototype) {
}

HRESULT CGorillaNamePlate::Render()
{
    if (m_fDissolve >= 0.999f) return S_OK;               

    if (FAILED(Bind_ShaderResources()))                   
        return E_FAIL;

    const _uint iNum = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNum; ++i)
    {
        _float4 vColor = s_vColorBlack;
        _float3 vMRA = { 0.f, 1.f, 1.f };
        if (i == 3)
            vColor = s_vColorWhite;
        else if (i == 2)
            vColor = s_vColorRed;
        else if (i == 0)
            vMRA = { 1.f, 0.f, 1.f };

        if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &vColor, sizeof(vColor)))) return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMRA", &vMRA, sizeof(vMRA))))   return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(14)))               
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

CGorillaNamePlate* CGorillaNamePlate::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGorillaNamePlate* pInstance = new CGorillaNamePlate(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CGorillaNamePlate");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CGorillaNamePlate::Clone(void* pArg)
{
    CGorillaNamePlate* pInstance = new CGorillaNamePlate(*this);
    if (FAILED(pInstance->Initialize(pArg)))               
    {
        MSG_BOX("Failed to Cloned : CGorillaNamePlate");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGorillaNamePlate::Free() { __super::Free(); }