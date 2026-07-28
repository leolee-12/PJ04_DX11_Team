#include "Kirby_ToyHat.h"

#include "GameInstance.h"

CKirby_ToyHat::CKirby_ToyHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_ToyHat::CKirby_ToyHat(const CKirby_ToyHat& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_ToyHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_ToyHat::Initialize(void* pArg)
{
    KIRBY_TOYHAT_DESC* pDesc = static_cast<KIRBY_TOYHAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_eOffMesh = TOY_HAT_MESH::HOVERING_STRAP;
    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

HRESULT CKirby_ToyHat::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i == m_eOffMesh)
            continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR))))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

_bool CKirby_ToyHat::Should_RenderShadowMesh(_uint iMeshIndex)
{
    switch (iMeshIndex)
    {
    case TOY_HAT_MESH::HOVERING_STRAP:
        return m_eOffMesh != TOY_HAT_MESH::HOVERING_STRAP;

    case TOY_HAT_MESH::STRAP:
        return m_eOffMesh != TOY_HAT_MESH::STRAP;

    case TOY_HAT_MESH::HEAD:
        return true;
    }

    return false;
}

void CKirby_ToyHat::Set_PartMode(CKirby* pKirby, KIRBY_PART_MODE ePartMode)
{
    switch (ePartMode)
    {
        case KIRBY_PART_MODE::HOVERING:
        {
            m_eOffMesh = TOY_HAT_MESH::STRAP;
            break;
        }
        case KIRBY_PART_MODE::DEFAULT:
        default:
        {
            m_eOffMesh = TOY_HAT_MESH::HOVERING_STRAP;

            break;
        }
    }
}

HRESULT CKirby_ToyHat::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_ToyHat");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_ToyHat* CKirby_ToyHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_ToyHat* pInstance = new CKirby_ToyHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_ToyHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_ToyHat::Clone(void* pArg)
{
    CKirby_ToyHat* pInstance = new CKirby_ToyHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_ToyHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_ToyHat::Free()
{
    __super::Free();
}
