#include "GigantEdge_Shield.h"
#include "Animator.h"

CGigantEdge_Shield::CGigantEdge_Shield(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}

CGigantEdge_Shield::CGigantEdge_Shield(const CGigantEdge_Shield& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CGigantEdge_Shield::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CGigantEdge_Shield::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    m_pAnimatorCom->Play("Reset");
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