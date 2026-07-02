#include "Kirby_SwordHat.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_SwordHat::CKirby_SwordHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_SwordHat::CKirby_SwordHat(const CKirby_SwordHat& Prototype)
    : CKirby_OnOffPart(Prototype) {
}

HRESULT CKirby_SwordHat::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_SwordHat::Initialize(void* pArg)
{
    KIRBY_SWORDHAT_DESC* pDesc = static_cast<KIRBY_SWORDHAT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_pAnimatorCom->Play("Deform", true, true);

    return S_OK;
}

void CKirby_SwordHat::Priority_Update(_float fTimeDelta)
{
    if (m_bOn == false)
        return;
}

void CKirby_SwordHat::Update(_float fTimeDelta)
{
    if (m_bOn == false)
        return;

    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_SwordHat::Late_Update(_float fTimeDelta)
{
    if (m_bOn == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CKirby_SwordHat::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIdx = 1;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPassIdx)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CKirby_SwordHat::Ready_Components()
{
    /* For.Com_Shader */
    m_pShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_SwordHat"), TEXT("Com_Model"));
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

HRESULT CKirby_SwordHat::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CKirby_SwordHat* CKirby_SwordHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_SwordHat* pInstance = new CKirby_SwordHat(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_SwordHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_SwordHat::Clone(void* pArg)
{
    CKirby_SwordHat* pInstance = new CKirby_SwordHat(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_SwordHat");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_SwordHat::Free()
{
    __super::Free();
}