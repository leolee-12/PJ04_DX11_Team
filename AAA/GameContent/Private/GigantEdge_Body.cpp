#include "GigantEdge_Body.h"
#include "Animator.h"

CGigantEdge_Body::CGigantEdge_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}

CGigantEdge_Body::CGigantEdge_Body(const CGigantEdge_Body& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CGigantEdge_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CGigantEdge_Body::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    if (FAILED(Ready_Components()))        return E_FAIL;

    m_pAnimatorCom->Play("Anger", true, true);
    return S_OK;
}

HRESULT CGigantEdge_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_GigantEdge_Body");
    t.szAnimEventFile = TEXT("../../Resources/YSH/MiniBoss/GigantEdge/Model/GigantEdge_anim_events.json");
    return Ready_MeshPart(t);
}

CGigantEdge_Body* CGigantEdge_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge_Body* pInstance = new CGigantEdge_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CGigantEdge_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGigantEdge_Body* CGigantEdge_Body::Clone(void* pArg)
{
    CGigantEdge_Body* pInstance = new CGigantEdge_Body(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CGigantEdge_Body"); Safe_Release(pInstance);
    }
    return pInstance;
}

void CGigantEdge_Body::Free() { __super::Free(); }