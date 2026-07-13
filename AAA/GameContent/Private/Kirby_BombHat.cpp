#include "Kirby_BombHat.h"

CKirby_BombHat::CKirby_BombHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_BombHat::CKirby_BombHat(const CKirby_BombHat& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_BombHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_BombHat::Initialize(void* pArg)
{
    KIRBY_BOMB_HAT_DESC* pDesc = static_cast<KIRBY_BOMB_HAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

HRESULT CKirby_BombHat::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIndex = ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR);

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPassIndex)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    
    return S_OK;
}

HRESULT CKirby_BombHat::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC°¡ tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_BombHat");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_BombHat* CKirby_BombHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_BombHat* pInstance = new CKirby_BombHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_BombHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_BombHat::Clone(void* pArg)
{
    CKirby_BombHat* pInstance = new CKirby_BombHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_BombHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_BombHat::Free()
{
    __super::Free();
}
