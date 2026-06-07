#include "UI_GenericContainer.h"
#include "UIPartObject.h"

CUI_GenericContainer::CUI_GenericContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject{ pDevice, pContext }
{
}

CUI_GenericContainer::CUI_GenericContainer(
    const CUI_GenericContainer& Prototype)
    : CUIContainerObject{ Prototype }
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

    return S_OK;
}

HRESULT CUI_GenericContainer::Remove_Part(const _wstring& strPartTag)
{
    auto it = m_UIPartObjects.find(strPartTag);
    if (it == m_UIPartObjects.end())
        return E_FAIL;

    Safe_Release(it->second);
    m_UIPartObjects.erase(it);
    m_UIPartPrototypeInfos.erase(strPartTag);
    return S_OK;
}

CUI_GenericContainer* CUI_GenericContainer::Create(
    ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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