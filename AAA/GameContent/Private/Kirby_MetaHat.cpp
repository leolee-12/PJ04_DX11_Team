#include "Kirby_MetaHat.h"

#include "GameInstance.h"

CKirby_MetaHat::CKirby_MetaHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_MetaHat::CKirby_MetaHat(const CKirby_MetaHat& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_MetaHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_MetaHat::Initialize(void* pArg)
{
    KIRBY_METAHAT_DESC* pDesc = static_cast<KIRBY_METAHAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

HRESULT CKirby_MetaHat::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        const _bool bMask = (i == 0 || i == 2);   // 가면(눈 오버레이) 메쉬

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        _uint iPassIndex = ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR);

        if (bMask)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MetaEyeTexture", i, MTEX_TYPE::UNKNOWN, 0)))
                return E_FAIL;
            iPassIndex = ETOUI(KIRBY_SHADER_PASS::META_MASK);
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPassIndex)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
    return S_OK;
}

HRESULT CKirby_MetaHat::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_MetaHat");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_MetaHat* CKirby_MetaHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_MetaHat* pInstance = new CKirby_MetaHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_MetaHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_MetaHat::Clone(void* pArg)
{
    CKirby_MetaHat* pInstance = new CKirby_MetaHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_MetaHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_MetaHat::Free()
{
    __super::Free();
}