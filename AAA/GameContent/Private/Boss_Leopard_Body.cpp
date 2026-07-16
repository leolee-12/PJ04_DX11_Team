#include "Boss_Leopard_Body.h"
#include "Animator.h"
#include "Model.h"
#include "Shader.h"
#include "GameContent_const.h"

CBoss_Leopard_Body::CBoss_Leopard_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMultiHitBoxPart(pDevice, pContext) {
}
CBoss_Leopard_Body::CBoss_Leopard_Body(const CBoss_Leopard_Body& Prototype)
    : CMultiHitBoxPart(Prototype) {
}

HRESULT CBoss_Leopard_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    Add_HitBox(LHB_DROP, "TopL", COLLIDER::SPHERE, 0.75f, 0.f, 10.f, 5.f);
    Add_HitBox(LHB_LCLAW, "LHaveL", COLLIDER::SPHERE, 1.f, 0.f, 10.f, 5.f);
    Add_HitBox(LHB_RCLAW, "RHaveL", COLLIDER::SPHERE, 1.f, 0.f, 10.f, 5.f);
    Add_HitBox(LHB_ASSAULT, "C_BustFurJ", COLLIDER::SPHERE, 2.f, 0.f, 10.f, 5.f);

    Enable_AllHitBoxes(false);

    m_pAnimatorCom->Play("Wait", false);
    m_iShadowPassIdx = PASS_SHADOW;
    return S_OK;
}

HRESULT CBoss_Leopard_Body::Render()
{
    if (FAILED(Bind_ShaderResources())) return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (!Is_MeshVisible(i))
            continue;

        const _uint iPass = Get_MeshPass(i);

        if (FAILED(Bind_MeshMaterials(i, iPass))) return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i))) return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPass))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CBoss_Leopard_Body::Render_Shadow()
{
    if (!m_bActive || nullptr == m_pModelCom) return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW)))) return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ)))) return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (!Is_MeshVisible(i)) continue;
        if (m_pAnimatorCom) m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
        if (FAILED(m_pShaderCom->Begin(m_iShadowPassIdx))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CBoss_Leopard_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Leopard;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.szAnimEventFile = TEXT("../../Resources/YSH/Boss/Leopard/Body/Leopard_anim_events.json");
    return Ready_MeshPart(t);
}

_bool CBoss_Leopard_Body::Is_MeshVisible(_uint iMesh) const
{
    switch (iMesh)
    {
        case 0: return true;                         

        case 1: case 2: return m_eEye == EYE::ANGRY;
        case 4: case 5: return m_eEye == EYE::NORMAL;
        case 6: case 7: return m_eEye == EYE::OPEN;
        case 3:         return m_eEye == EYE::CLOSE;  

        case 9: return m_eClaw == CLAW::NORMAL;       
        case 8: return m_eClaw == CLAW::ATTACK;       
    }
    return false;
}

_uint CBoss_Leopard_Body::Get_MeshPass(_uint iMesh) const
{
    switch (iMesh)
    {
        case 1: case 4: case 6: return PASS_EYE;   
        default:                return PASS_BODY;  
    }
}

HRESULT CBoss_Leopard_Body::Bind_MeshMaterials(_uint iMeshIndex, _uint iPass)
{
    switch (iPass)
    {
        case PASS_BODY:
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::DIFFUSE, 0)))   return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0)))    return E_FAIL;
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", iMeshIndex, MTEX_TYPE::METALNESS, 0);

        case PASS_EYE:
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 0)))   return E_FAIL;
            return m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0);
    }
    return S_OK;
}

CBoss_Leopard_Body* CBoss_Leopard_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Leopard_Body* p = new CBoss_Leopard_Body(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created: CBoss_Leopard_Body"); Safe_Release(p); }
    return p;
}
CBoss_Leopard_Body* CBoss_Leopard_Body::Clone(void* pArg)
{
    CBoss_Leopard_Body* p = new CBoss_Leopard_Body(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned: CBoss_Leopard_Body"); Safe_Release(p); }
    return p;
}
void CBoss_Leopard_Body::Free() { __super::Free(); }