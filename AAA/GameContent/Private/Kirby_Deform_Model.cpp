#include "Kirby_Deform_Model.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_Deform_Model::CKirby_Deform_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_Deform_Model::CKirby_Deform_Model(const CKirby_Deform_Model& Prototype)
    : CPartObject(Prototype) {
}

HRESULT CKirby_Deform_Model::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Deform_Model::Initialize(void* pArg)
{
    KIRBY_FORM_DESC* pDesc = static_cast<KIRBY_FORM_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CKirby_Deform_Model::Priority_Update(_float fTimeDelta)
{
}

void CKirby_Deform_Model::Update(_float fTimeDelta)
{
}

void CKirby_Deform_Model::Late_Update(_float fTimeDelta)
{
}

HRESULT CKirby_Deform_Model::Render()
{
    return S_OK;
}

const _float4x4* CKirby_Deform_Model::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    if (m_pModelCom == nullptr)
    {
        MSG_BOX("m_pModelCom is nullptr: Kirby_Form");
        return nullptr;
    }

    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

const _float4x4* CKirby_Deform_Model::Get_HatBoneMatirx()
{
    return Get_BoneMatrixPtr("HatL");;
}

HRESULT CKirby_Deform_Model::Bind_ShaderResources(CShader* pShader)
{
    if (pShader == nullptr)
    {
        MSG_BOX("pShader is nullptr: Kirby_Form");
        return E_FAIL;
    }

    if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_Deform_Model::Render_PBRMesh(_uint iMeshIndex)
{
    if (m_pPBRShaderCom == nullptr)
    {
        MSG_BOX("m_pPBRShaderCom is nullptr: Kirby_Form");
        return E_FAIL;
    }

    if (FAILED(m_pModelCom->Bind_Material(m_pPBRShaderCom, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::DIFFUSE, 0)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pPBRShaderCom, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pPBRShaderCom, "g_MRATexture", iMeshIndex, MTEX_TYPE::METALNESS, 0)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pPBRShaderCom, "g_BoneMatrices", iMeshIndex)))
        return E_FAIL;

    if (FAILED(m_pPBRShaderCom->Begin(1)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Render(iMeshIndex)))
        return E_FAIL;

    return S_OK;
}

void CKirby_Deform_Model::Free()
{
    __super::Free();
}