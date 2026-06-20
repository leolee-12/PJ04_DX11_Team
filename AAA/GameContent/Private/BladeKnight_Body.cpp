#include "BladeKnight_Body.h"
#include "Animator.h"

CBladeKnight_Body::CBladeKnight_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart{ pDevice, pContext } {
}

CBladeKnight_Body::CBladeKnight_Body(const CBladeKnight_Body& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CBladeKnight_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CBladeKnight_Body::Initialize(void* pArg)
{
    auto pDesc = static_cast<BLADEKNIGHT_BODY_DESC*>(pArg);
    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc))) return E_FAIL;
    if (FAILED(Ready_Components()))         return E_FAIL;

    m_pAnimatorCom->Play("Thrust", true, true);
    return S_OK;
}

HRESULT CBladeKnight_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_AnimMesh_PBR;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_BladeKnight_Body");
    t.szAnimEventFile = TEXT("../../Resources/CHJ/Monster/BladeKnight/BladeKnight_Anim_Events.json");
    return Ready_MeshPart(t);
}

CBladeKnight_Body* CBladeKnight_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBladeKnight_Body* pInstance = new CBladeKnight_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBladeKnight_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBladeKnight_Body::Clone(void* pArg)
{
    CBladeKnight_Body* pInstance = new CBladeKnight_Body(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBladeKnight_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBladeKnight_Body::Free() { __super::Free(); }