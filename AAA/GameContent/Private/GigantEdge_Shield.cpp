#include "GigantEdge_Shield.h"
#include "Animator.h"
#include "Shader_PassMeta.h"

CGigantEdge_Shield::CGigantEdge_Shield(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterHitPart(pDevice, pContext) {
}

CGigantEdge_Shield::CGigantEdge_Shield(const CGigantEdge_Shield& Prototype)
    : CMonsterHitPart(Prototype) {
}

HRESULT CGigantEdge_Shield::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))            
        return E_FAIL;
    if (FAILED(Ready_Components()))                   
        return E_FAIL;

    CAPSULE_DESC CapsuleDesc{};
    CapsuleDesc.fHeight = 1.f;
    CapsuleDesc.fRadius = 0.25f;
    if (FAILED(Ready_HitBox(CapsuleDesc)))
        return E_FAIL;

    m_iShadowPassIdx = ETOI(ANIM_MESH_PASS::SHADOW);

    m_pAnimatorCom->Play("Reset");
    Set_Drawn(false);                     
    return S_OK;
}

HRESULT CGigantEdge_Shield::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_GigantEdge_Shield");
    return Ready_MeshPart(t);
}

CGigantEdge_Shield* CGigantEdge_Shield::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge_Shield* pInstance = new CGigantEdge_Shield(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CGigantEdge_Shield");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGigantEdge_Shield* CGigantEdge_Shield::Clone(void* pArg)
{
    CGigantEdge_Shield* pInstance = new CGigantEdge_Shield(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CGigantEdge_Shield");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGigantEdge_Shield::Free() { __super::Free(); }