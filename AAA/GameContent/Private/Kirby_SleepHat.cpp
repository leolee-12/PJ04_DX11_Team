#include "Kirby_SleepHat.h"

CKirby_SleepHat::CKirby_SleepHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_SleepHat::CKirby_SleepHat(const CKirby_SleepHat& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_SleepHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_SleepHat::Initialize(void* pArg)
{
    KIRBY_SLEEP_HAT_DESC* pDesc = static_cast<KIRBY_SLEEP_HAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

HRESULT CKirby_SleepHat::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
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

HRESULT CKirby_SleepHat::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC\uAC00 tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_SleepHat");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_SleepHat* CKirby_SleepHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_SleepHat* pInstance = new CKirby_SleepHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_SleepHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_SleepHat::Clone(void* pArg)
{
    CKirby_SleepHat* pInstance = new CKirby_SleepHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_SleepHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_SleepHat::Free()
{
    __super::Free();
}
