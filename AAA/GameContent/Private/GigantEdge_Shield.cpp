#include "GigantEdge_Shield.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "Animator.h"

CGigantEdge_Shield::CGigantEdge_Shield(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CGigantEdge_Shield::CGigantEdge_Shield(const CGigantEdge_Shield& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CGigantEdge_Shield::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CGigantEdge_Shield::Initialize(void* pArg)
{
    GIGANTEDGE_SHIELD_DESC* pDesc = static_cast<GIGANTEDGE_SHIELD_DESC*>(pArg);

    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Reset");

    return S_OK;
}

void CGigantEdge_Shield::Priority_Update(_float fTimeDelta)
{
}

void CGigantEdge_Shield::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CGigantEdge_Shield::Late_Update(_float fTimeDelta)
{
    // 부모 월드 * 소켓 본
    __super::Compute_CombinedWorldMatrix(
        XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
        XMLoadFloat4x4(m_pSocketBoneMatrix));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CGigantEdge_Shield::Render()
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

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CGigantEdge_Shield::Render_Shadow()
{
    return S_OK;
}

HRESULT CGigantEdge_Shield::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag,
        TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_GigantEdge_Shield"),
        TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CGigantEdge_Shield::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CGigantEdge_Shield* CGigantEdge_Shield::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge_Shield* pInstance = new CGigantEdge_Shield(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGigantEdge_Shield");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGigantEdge_Shield* CGigantEdge_Shield::Clone(void* pArg)
{
    CGigantEdge_Shield* pInstance = new CGigantEdge_Shield(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CGigantEdge_Shield");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGigantEdge_Shield::Free()
{
    __super::Free();
}