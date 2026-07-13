#include "Kirby_IceHat.h"

CKirby_IceHat::CKirby_IceHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_IceHat::CKirby_IceHat(const CKirby_IceHat& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_IceHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_IceHat::Initialize(void* pArg)
{
    KIRBY_ICE_HAT_DESC* pDesc = static_cast<KIRBY_ICE_HAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

HRESULT CKirby_IceHat::Render()
{
    // ¿”Ω√

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    static constexpr _float4 vIceDiffuse = { 0.70f, 0.84f, 0.95f, 1.f };
    static constexpr _float3 vIceMRA = { 0.f, 0.15f, 1.f };
    static constexpr _float4 vIceEmissive = { 0.f, 0.f, 0.f, 1.f };

    static constexpr _float4 vJuelDiffuse = { 0.45f, 0.78f, 0.95f, 1.f };
    static constexpr _float3 vJuelMRA = { 0.10f, 0.10f, 1.f };
    static constexpr _float4 vJuelEmissive = { 0.03f, 0.06f, 0.09f, 1.f };

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIndex = ETOUI(KIRBY_SHADER_PASS::ANIM_CONSTANT_PBR);

        if (i == 0u)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 1)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPassIndex = ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR);
        }
        else
        {
            const _bool bJuel = (i == 3u);
            const _float4* pDiffuse = bJuel ? &vJuelDiffuse : &vIceDiffuse;
            const _float3* pMRA = bJuel ? &vJuelMRA : &vIceMRA;
            const _float4* pEmissive = bJuel ? &vJuelEmissive : &vIceEmissive;

            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", pDiffuse, sizeof(_float4))))
                return E_FAIL;
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantMRA", pMRA, sizeof(_float3))))
                return E_FAIL;
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantEmissive", pEmissive, sizeof(_float4))))
                return E_FAIL;
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(iPassIndex)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CKirby_IceHat::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC∞° tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_IceHat");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_IceHat* CKirby_IceHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_IceHat* pInstance = new CKirby_IceHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_IceHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_IceHat::Clone(void* pArg)
{
    CKirby_IceHat* pInstance = new CKirby_IceHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_IceHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_IceHat::Free()
{
    __super::Free();
}
