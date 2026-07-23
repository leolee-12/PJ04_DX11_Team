#include "Kirby_ToyHammer.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_ToyHammer::CKirby_ToyHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_ToyHammer::CKirby_ToyHammer(const CKirby_ToyHammer& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_ToyHammer::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_ToyHammer::Initialize(void* pArg)
{
    KIRBY_TOYHAMMER_DESC* pDesc = static_cast<KIRBY_TOYHAMMER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Reset", true, true);

    return S_OK;
}

HRESULT CKirby_ToyHammer::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (m_bBurn && (i == TOY_HAMMER_MESH::HEAD || i == TOY_HAMMER_MESH::TOP))
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

void CKirby_ToyHammer::Set_PartMode(CKirby* pKirby, KIRBY_PART_MODE ePartMode)
{
    switch (ePartMode)
    {
        case KIRBY_PART_MODE::BACK:
        {
            Set_SocketBoneMatrix(pKirby->Get_Body()->Get_BoneMatrixPtr("CenterL"));
            m_pAnimatorCom->Play("Carry", true, true, 0.f);
            break;
        }
        case KIRBY_PART_MODE::DEFAULT:
        default:
        {
            Set_SocketBoneMatrix(pKirby->Get_Body()->Get_BoneMatrixPtr("RHaveL"));
            m_pAnimatorCom->Play("Reset", true, true, 0.f);
            break;
        }
    }
}

HRESULT CKirby_ToyHammer::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_ToyHammer");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

CKirby_ToyHammer* CKirby_ToyHammer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_ToyHammer* pInstance = new CKirby_ToyHammer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_ToyHammer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_ToyHammer::Clone(void* pArg)
{
    CKirby_ToyHammer* pInstance = new CKirby_ToyHammer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_ToyHammer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_ToyHammer::Free()
{
    __super::Free();
}