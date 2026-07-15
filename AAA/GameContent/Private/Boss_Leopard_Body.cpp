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

    // TODO: 레오파드 리그에 맞는 본/사이즈로 조정 (지금은 몸통 허트/샘플 하나)
    Add_HitBox(LHB_BODY, "Spine1J", COLLIDER::SPHERE, 4.f, 0.f, 0.f, 0.f);

    m_pAnimatorCom->Play("Wait", false);   // TODO: 레오파드 대기 클립명 확인
    m_iShadowPassIdx = PASS_SHADOW;
    return S_OK;
}

HRESULT CBoss_Leopard_Body::Render()
{
    if (FAILED(Bind_ShaderResources())) return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0))) return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i))) return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(PASS_BODY))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CBoss_Leopard_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;   // TODO: 전용 Shader_Leopard 생기면 교체
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.szAnimEventFile = TEXT("../../Resources/YSH/Boss/Leopard/Body/Leopard_anim_events.json");
    return Ready_MeshPart(t);
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