#include "MetaknightNamePlate.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Model.h"

CMetaknightNamePlate::CMetaknightNamePlate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBossNamePlate(pDevice, pContext) {
}
CMetaknightNamePlate::CMetaknightNamePlate(const CMetaknightNamePlate& Prototype)
    : CBossNamePlate(Prototype) {
}

HRESULT CMetaknightNamePlate::Ready_Events()
{
    Set_Active(false);

    Subscribe_Event(Get_ActivateEventTag(), [this](void* p) {
        m_pAnchor = static_cast<const _float4x4*>(p);
        Activate();
        });

    return S_OK;
}

HRESULT CMetaknightNamePlate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_fHoldTime = 2.7f;

    m_fFadeTime = 0.3f;

    return S_OK;
}

void CMetaknightNamePlate::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (m_pAnchor)
    {
        _matrix matParent = XMLoadFloat4x4(m_pAnchor);
        matParent.r[0] = XMVector3Normalize(matParent.r[0]);
        matParent.r[1] = XMVector3Normalize(matParent.r[1]);
        matParent.r[2] = XMVector3Normalize(matParent.r[2]);

        _matrix matLocal = XMMatrixScaling(BONE_SCALE, BONE_SCALE, BONE_SCALE);
        matLocal.r[3] = XMVectorSet(BONE_OFFSET.x, BONE_OFFSET.y, BONE_OFFSET.z, 1.f);

        m_pTransformCom->Set_WorldMatrix(matLocal * matParent);
    }

    __super::Late_Update(fTimeDelta);
}

HRESULT CMetaknightNamePlate::Render()
{
    if (m_fDissolve >= 0.999f) return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNum = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNum; ++i)
    {
        _float4 vColor;
        _float3 vMRA;

        switch (i)
        {
            case 0:
            case 1:
                vColor = s_vColorBlack;
                vMRA = { 0.f, 0.f, 1.f };
                break;
            case 2:
                vColor = s_vColorBlue;
                vMRA = { 0.f, 1.f, 1.f };
                break;
            default:
                vColor = s_vColorWhite;
                vMRA = { 0.f, 1.f, 1.f };
                break;
        }

        if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &vColor, sizeof(vColor)))) return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMRA", &vMRA, sizeof(vMRA))))       return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(14)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

CMetaknightNamePlate* CMetaknightNamePlate::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMetaknightNamePlate* pInstance = new CMetaknightNamePlate(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMetaknightNamePlate");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMetaknightNamePlate::Clone(void* pArg)
{
    CMetaknightNamePlate* pInstance = new CMetaknightNamePlate(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMetaknightNamePlate");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMetaknightNamePlate::Free() { __super::Free(); }