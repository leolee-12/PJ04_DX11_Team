#include "Kirby_Sword.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_Sword::CKirby_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_Sword::CKirby_Sword(const CKirby_Sword& Prototype)
    : CKirby_OnOffPart(Prototype) {
}

HRESULT CKirby_Sword::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Sword::Initialize(void* pArg)
{
    KIRBY_SWORD_DESC* pDesc = static_cast<KIRBY_SWORD_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Reset", true, true);

    return S_OK;
}

void CKirby_Sword::Priority_Update(_float fTimeDelta)
{
    if (m_bOn == false)
        return;
}

void CKirby_Sword::Update(_float fTimeDelta)
{
    if (m_bOn == false)
        return;

    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_Sword::Late_Update(_float fTimeDelta)
{
    if (m_bOn == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CKirby_Sword::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIdx{};

        if (i != 1) {

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
            m_pShaderCom->Bind_RawValue("g_vConstantDiffuse",   &m_vConstantDiffuse, sizeof(_float4));
            m_pShaderCom->Bind_RawValue("g_vConstantMRA",       &m_vConstantMRA, sizeof(_float3));
            m_pShaderCom->Bind_RawValue("g_vConstantEmissive",  &m_vConstantEmissive, sizeof(_float4));

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

HRESULT CKirby_Sword::Ready_Components()
{
    /* For.Com_Shader */
    m_pShaderCom = Add_Component<CShader>(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_Sword"), TEXT("Com_Model"));
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

HRESULT CKirby_Sword::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CKirby_Sword* CKirby_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_Sword* pInstance = new CKirby_Sword(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_Sword::Clone(void* pArg)
{
    CKirby_Sword* pInstance = new CKirby_Sword(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Sword::Free()
{
    __super::Free();
}