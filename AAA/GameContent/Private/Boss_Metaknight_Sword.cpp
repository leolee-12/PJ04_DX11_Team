#include "Boss_Metaknight_Sword.h"
#include "GameInstance.h"

CBoss_Metaknight_Sword::CBoss_Metaknight_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterHitPart(pDevice, pContext) {
}
CBoss_Metaknight_Sword::CBoss_Metaknight_Sword(const CBoss_Metaknight_Sword& Prototype)
    : CMonsterHitPart(Prototype) {
}

HRESULT CBoss_Metaknight_Sword::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))  return E_FAIL;
    if (FAILED(Ready_Components()))         return E_FAIL;

    // TODO: 실측 후 튜닝 (갈락시아는 GigantEdge 검보다 작음)
    CAPSULE_DESC CapsuleDesc{};
    CapsuleDesc.vCenter = { 0.f, 0.f, 0.f };
    CapsuleDesc.fHeight = 1.2f;
    CapsuleDesc.fRadius = 0.35f;
    CapsuleDesc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f };
    if (FAILED(Ready_HitBox(CapsuleDesc)))
        return E_FAIL;

    m_iShadowPassIdx = 6;

    return S_OK;
}

HRESULT CBoss_Metaknight_Sword::Render()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_CrystalTexture", i, MTEX_TYPE::UNKNOWN, 1)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_HeightTexture", i, MTEX_TYPE::UNKNOWN, 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(7)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBoss_Metaknight_Sword::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Metaknight;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = false;
    return Ready_MeshPart(t);
}

CBoss_Metaknight_Sword* CBoss_Metaknight_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Metaknight_Sword* pInstance = new CBoss_Metaknight_Sword(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Metaknight_Sword* CBoss_Metaknight_Sword::Clone(void* pArg)
{
    CBoss_Metaknight_Sword* pInstance = new CBoss_Metaknight_Sword(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Metaknight_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_Sword::Free() { __super::Free(); }