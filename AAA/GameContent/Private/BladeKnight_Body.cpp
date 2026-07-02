#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Shader.h"
#include "Model.h"

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

HRESULT CBladeKnight_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_MaskTexture", i, MTEX_TYPE::NORMALS, 1);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture1", i, MTEX_TYPE::NORMALS, 2);

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(5)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CBladeKnight_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_BladeKnight_Body");
    t.szAnimEventFile = TEXT("../../Resources/CHJ/Monster/BladeKnight/BladeKnight_AnimEvents.json");
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