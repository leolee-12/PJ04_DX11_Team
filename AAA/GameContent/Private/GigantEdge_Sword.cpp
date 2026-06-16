#include "GigantEdge_Sword.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "Animator.h"

CGigantEdge_Sword::CGigantEdge_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CGigantEdge_Sword::CGigantEdge_Sword(const CGigantEdge_Sword& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CGigantEdge_Sword::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CGigantEdge_Sword::Initialize(void* pArg)
{
    GIGANTEDGE_SWORD_DESC* pDesc = static_cast<GIGANTEDGE_SWORD_DESC*>(pArg);

    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CGigantEdge_Sword::Priority_Update(_float fTimeDelta)
{
}

void CGigantEdge_Sword::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;
}

void CGigantEdge_Sword::Late_Update(_float fTimeDelta)
{
    // 부모 월드 * 소켓 본
    __super::Compute_CombinedWorldMatrix(
        XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
        XMLoadFloat4x4(m_pSocketBoneMatrix));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CGigantEdge_Sword::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        // TODO: 머티리얼 바인딩
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CGigantEdge_Sword::Render_Shadow()
{
    return S_OK;
}

HRESULT CGigantEdge_Sword::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag,
        TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_GigantEdge_Sword"),
        TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CGigantEdge_Sword::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CGigantEdge_Sword* CGigantEdge_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge_Sword* pInstance = new CGigantEdge_Sword(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGigantEdge_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGigantEdge_Sword* CGigantEdge_Sword::Clone(void* pArg)
{
    CGigantEdge_Sword* pInstance = new CGigantEdge_Sword(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CGigantEdge_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGigantEdge_Sword::Free()
{
    __super::Free();
}