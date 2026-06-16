#include "BladeKnight_Sword.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Animator.h"

CBladeKnight_Sword::CBladeKnight_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CBladeKnight_Sword::CBladeKnight_Sword(const CBladeKnight_Sword& Prototype)
    : CPartObject( Prototype )
{
}

HRESULT CBladeKnight_Sword::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;

    return S_OK;
}

HRESULT CBladeKnight_Sword::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    BLADEKNIGHT_SWORD_DESC* pDesc = static_cast<BLADEKNIGHT_SWORD_DESC*>(pArg);

    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;
    if (nullptr == m_pSocketBoneMatrix)
        return E_FAIL;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Thrust", false , true);

    return S_OK;
}

void CBladeKnight_Sword::Priority_Update(_float fTimeDelta)
{
}

void CBladeKnight_Sword::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    if (nullptr != m_pAnimatorCom)
        m_pAnimatorCom->Update(fTimeDelta);
}

void CBladeKnight_Sword::Late_Update(_float fTimeDelta)
{
    if (nullptr == m_pSocketBoneMatrix)
        return;

    __super::Compute_CombinedWorldMatrix(
        XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
        XMLoadFloat4x4(m_pSocketBoneMatrix));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CBladeKnight_Sword::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIdx{};

        if (i != 1)
        {
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPassIdx = 1;
        }
        else
        {
            m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", &m_vConstantDiffuse, sizeof(_float4));
            m_pShaderCom->Bind_RawValue("g_vConstantMRA", &m_vConstantMRA, sizeof(_float3));
            m_pShaderCom->Bind_RawValue("g_vConstantEmissive", &m_vConstantEmissive, sizeof(_float4));
        
            iPassIdx = 3;

        }
     

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPassIdx)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CBladeKnight_Sword::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(
        Shader_AnimMesh_PBR.iLevelID,
        Shader_AnimMesh_PBR.szProtoTag,
        TEXT("Com_Shader"));

    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(
        m_iPrototypeLevel,
        TEXT("Prototype_Component_Model_BladeKnight_Sword"),
        TEXT("Com_Model"));

    if (nullptr == m_pModelCom)
        return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;

    m_pAnimatorCom = Add_Component<CAnimator>(
        TEXT("Com_Animator"),
        CAnimator::Create(m_pDevice, m_pContext));

    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CBladeKnight_Sword::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix",
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix",
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CBladeKnight_Sword* CBladeKnight_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBladeKnight_Sword* pInstance = new CBladeKnight_Sword(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBladeKnight_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBladeKnight_Sword::Clone(void* pArg)
{
    CBladeKnight_Sword* pInstance = new CBladeKnight_Sword(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBladeKnight_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBladeKnight_Sword::Free()
{
    __super::Free();
}