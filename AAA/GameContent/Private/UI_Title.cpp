#include "UI_Title.h"

CUI_Title::CUI_Title(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIContainerObject{ pDevice, pContext }
{

}

CUI_Title::CUI_Title(const CUI_Title& Prototype)
	: CUIContainerObject { Prototype }
{

}

HRESULT CUI_Title::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_Title::Initialize(void* pArg)
{
    UI_TITLE_DESC Default{};
    UI_TITLE_DESC* pDesc = pArg ? static_cast<UI_TITLE_DESC*>(pArg) : &Default;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (pDesc->bCreateTitleImage)
    {
        if (FAILED(Add_UIPartObject(ETOUI(LEVEL::STATIC), CUI_Image::PROTOTYPE_TAG, 
                                    pDesc->szTitleImagePartTag ? pDesc->szTitleImagePartTag : PART_TAG_TITLE_IMAGE, &pDesc->TitleImageDesc)))
            return E_FAIL;
    }

    return S_OK;
}


CUI_Title* CUI_Title::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Title* pInstance = new CUI_Title(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Title");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Title::Clone(void* pArg)
{
    CUI_Title* pInstance = new CUI_Title(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Title");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Title::Free()
{
    __super::Free();
}
