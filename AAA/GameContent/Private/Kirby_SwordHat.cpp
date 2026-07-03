#include "Kirby_SwordHat.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_SwordHat::CKirby_SwordHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_SwordHat::CKirby_SwordHat(const CKirby_SwordHat& Prototype)
    : CKirby_OnOffPart(Prototype) {
}

HRESULT CKirby_SwordHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_SwordHat::Initialize(void* pArg)
{
    KIRBY_SWORDHAT_DESC* pDesc = static_cast<KIRBY_SWORDHAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

HRESULT CKirby_SwordHat::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIdx = 1;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPassIdx)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CKirby_SwordHat::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Kirby;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_SwordHat");
    t.bAnimated = true;
    return Ready_MeshPart(t);
}

CKirby_SwordHat* CKirby_SwordHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_SwordHat* pInstance = new CKirby_SwordHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_SwordHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_SwordHat::Clone(void* pArg)
{
    CKirby_SwordHat* pInstance = new CKirby_SwordHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_SwordHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_SwordHat::Free()
{
    __super::Free();
}