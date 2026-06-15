#include "UI_GenericContainer.h"
#include "UIPartObject.h"

CUI_GenericContainer::CUI_GenericContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject{ pDevice, pContext }
{
}

CUI_GenericContainer::CUI_GenericContainer(const CUI_GenericContainer& Prototype)
    : CUIContainerObject( Prototype )
{
}

HRESULT CUI_GenericContainer::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_GenericContainer::Initialize(void* pArg)
{
    UI_GENERIC_CONTAINER_DESC Default{};
    UI_GENERIC_CONTAINER_DESC* pDesc =
        pArg ? static_cast<UI_GENERIC_CONTAINER_DESC*>(pArg) : &Default;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    m_pUIAnimatorCom = Add_Component<CUIAnimatorCom>(
        TEXT("Com_UIAnimator"),
        CUIAnimatorCom::Create(m_pDevice, m_pContext));

    if (!m_pUIAnimatorCom || FAILED(m_pUIAnimatorCom->Initialize(nullptr)))
        return E_FAIL;

    Bind_UIAnimator();

    return S_OK;
}

void CUI_GenericContainer::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CUI_GenericContainer::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    if (m_pUIAnimatorCom)
        m_pUIAnimatorCom->Update(fTimeDelta);

    __super::Update(fTimeDelta);
}

void CUI_GenericContainer::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

void CUI_GenericContainer::On_Deserialized()
{
    Bind_UIAnimator();
}

void CUI_GenericContainer::On_UIPartsChanged()
{
    Bind_UIAnimator();
}

void CUI_GenericContainer::Bind_UIAnimator()
{
    if (m_pUIAnimatorCom)
        m_pUIAnimatorCom->Bind_Parts(m_UIPartObjects);
}

CUI_GenericContainer* CUI_GenericContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_GenericContainer* pInstance =
        new CUI_GenericContainer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_GenericContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_GenericContainer::Clone(void* pArg)
{
    CUI_GenericContainer* pInstance =
        new CUI_GenericContainer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_GenericContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_GenericContainer::Free()
{
    __super::Free();
}