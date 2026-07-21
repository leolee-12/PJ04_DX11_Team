#include "Boss_Metaknight_ReplicaSword.h"
#include "GameInstance.h"

CBoss_Metaknight_ReplicaSword::CBoss_Metaknight_ReplicaSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterHitPart(pDevice, pContext) {
}
CBoss_Metaknight_ReplicaSword::CBoss_Metaknight_ReplicaSword(const CBoss_Metaknight_ReplicaSword& Prototype)
    : CMonsterHitPart(Prototype) {
}

HRESULT CBoss_Metaknight_ReplicaSword::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))  return E_FAIL;
    if (FAILED(Ready_Components()))         return E_FAIL;

    CAPSULE_DESC CapsuleDesc{};
    CapsuleDesc.vCenter = { 0.f, 0.f, 0.f };
    CapsuleDesc.fHeight = 1.2f;
    CapsuleDesc.fRadius = 0.35f;
    CapsuleDesc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f };
    if (FAILED(Ready_HitBox(CapsuleDesc)))
        return E_FAIL;

    m_iShadowPassIdx = 2;

    return S_OK;
}

HRESULT CBoss_Metaknight_ReplicaSword::Render()
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

        // TODO: Crystal(UNKNOWN 1) 보석 이미시브, Height(UNKNOWN 0)는 전용 패스 만들 때
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBoss_Metaknight_ReplicaSword::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_NonAnimMesh_PBR;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = false;
    return Ready_MeshPart(t);
}

CBoss_Metaknight_ReplicaSword* CBoss_Metaknight_ReplicaSword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Metaknight_ReplicaSword* pInstance = new CBoss_Metaknight_ReplicaSword(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_ReplicaSword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Metaknight_ReplicaSword* CBoss_Metaknight_ReplicaSword::Clone(void* pArg)
{
    CBoss_Metaknight_ReplicaSword* pInstance = new CBoss_Metaknight_ReplicaSword(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Metaknight_ReplicaSword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_ReplicaSword::Free() { __super::Free(); }