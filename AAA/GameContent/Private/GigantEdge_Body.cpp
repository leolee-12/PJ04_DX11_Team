#include "GigantEdge_Body.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "Animator.h"

CGigantEdge_Body::CGigantEdge_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CGigantEdge_Body::CGigantEdge_Body(const CGigantEdge_Body& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CGigantEdge_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CGigantEdge_Body::Initialize(void* pArg)
{
    GIGANTEDGE_BODY_DESC* pDesc = static_cast<GIGANTEDGE_BODY_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    // TODO: 기본 애님 재생
    // m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

void CGigantEdge_Body::Priority_Update(_float fTimeDelta)
{
}

void CGigantEdge_Body::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CGigantEdge_Body::Late_Update(_float fTimeDelta)
{
    __super::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CGigantEdge_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        // TODO: 머티리얼 바인딩 (Diffuse / Normal / MRA 등)
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

HRESULT CGigantEdge_Body::Render_Shadow()
{
    return S_OK;
}

const _float4x4* CGigantEdge_Body::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CGigantEdge_Body::Ready_Components()
{
    /* For.Com_Shader */
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag,
        TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */ // TODO: 모델 프로토타입 태그 교체
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_GigantEdge_Body"),
        TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CGigantEdge_Body::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CGigantEdge_Body* CGigantEdge_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGigantEdge_Body* pInstance = new CGigantEdge_Body(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGigantEdge_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGigantEdge_Body* CGigantEdge_Body::Clone(void* pArg)
{
    CGigantEdge_Body* pInstance = new CGigantEdge_Body(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CGigantEdge_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGigantEdge_Body::Free()
{
    __super::Free();
}