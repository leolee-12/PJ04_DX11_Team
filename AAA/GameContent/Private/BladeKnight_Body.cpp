#include "BladeKnight_Body.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CBladeKnight_Body::CBladeKnight_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CBladeKnight_Body::CBladeKnight_Body(const CBladeKnight_Body& Prototype)
    : CPartObject ( Prototype )
{

}

HRESULT CBladeKnight_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;

    return S_OK;
}

HRESULT CBladeKnight_Body::Initialize(void* pArg)
{
    BLADEKNIGHT_BODY_DESC* pDesc = static_cast<BLADEKNIGHT_BODY_DESC*>(pArg);

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Thrust", true, true);

    return S_OK;
}

void CBladeKnight_Body::Priority_Update(_float fTimeDelta)
{
}

void CBladeKnight_Body::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CBladeKnight_Body::Late_Update(_float fTimeDelta)
{
    __super::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CBladeKnight_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; i++)
    {
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

const _float4x4* CBladeKnight_Body::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    if (nullptr == m_pModelCom || nullptr == pBoneName)
        return nullptr;

    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

HRESULT CBladeKnight_Body::Ready_Components()
{
    // 일반 스킨드 PBR
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_BladeKnight_Body"), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CBladeKnight_Body::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CBladeKnight_Body* CBladeKnight_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBladeKnight_Body* pInstance = new CBladeKnight_Body(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBladeKnight_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBladeKnight_Body::Clone(void* pArg)
{
    CBladeKnight_Body* pInstance = new CBladeKnight_Body(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBladeKnight_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBladeKnight_Body::Free()
{
	__super::Free();
}