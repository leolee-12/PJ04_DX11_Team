#include "UI_TestImageContainer.h"


CUI_TestImageContainer::CUI_TestImageContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject{ pDevice, pContext }
{
}

CUI_TestImageContainer::CUI_TestImageContainer(const CUI_TestImageContainer& Prototype)
    : CUIContainerObject{ Prototype }
{
}

HRESULT CUI_TestImageContainer::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_TestImageContainer::Initialize(void* pArg)
{
    UI_TESTIMAGE_CONTAINER_DESC Default{};
    UI_TESTIMAGE_CONTAINER_DESC* pDesc =
        pArg ? static_cast<UI_TESTIMAGE_CONTAINER_DESC*>(pArg) : &Default;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (pDesc->bCreateImagePart)
    {
        if (FAILED(Add_UIPartObject(
            ETOUI(LEVEL::STATIC),
            CUI_Image::PROTOTYPE_TAG,
            pDesc->szPartTag ? pDesc->szPartTag : L"Image",
            &pDesc->ImageDesc)))
            return E_FAIL;
    }

    return S_OK;
}

CUI_TestImageContainer* CUI_TestImageContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_TestImageContainer* pInstance = new CUI_TestImageContainer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_TestImageContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_TestImageContainer::Clone(void* pArg)
{
    CUI_TestImageContainer* pInstance = new CUI_TestImageContainer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_TestImageContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_TestImageContainer::Free()
{
    __super::Free();
}