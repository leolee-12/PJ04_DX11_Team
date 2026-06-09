#include "UI_Title.h"
#include "UI_Image.h"
#include "UI_Effect.h"
#include "UI_SpriteAnim.h"
#include "UI_Text.h"

CUI_Title::CUI_Title(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIContainerObject{ pDevice, pContext }
{

}

CUI_Title::CUI_Title(const CUI_Title& Prototype)
	: CUIContainerObject ( Prototype )
{

}

HRESULT CUI_Title::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_Title::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CUI_Title::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CUI_Title::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CUI_Title::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_Title::Ready_Events()
{
    if (FAILED(__super::Ready_Events()))
        return E_FAIL;

    return S_OK;
}

void CUI_Title::On_Deserialized()
{
    if (FAILED(Cache_Parts()))
        return;
}

void CUI_Title::On_UIPartsChanged()
{
    Cache_Parts();
}

HRESULT CUI_Title::Cache_Parts()
{
    //m_pIcon = nullptr;
    //m_pSpark = nullptr;
    //m_pAmount = nullptr;

    auto FindPart = [this](const _wstring& strTag) -> CUIPartObject*
        {
            auto it = m_UIPartObjects.find(strTag);
            if (it == m_UIPartObjects.end())
                return nullptr;

            return it->second;
        };

    //m_pIcon = dynamic_cast<CUI_Image*>(FindPart(L"Part_PointStar_Image"));
    //if (!m_pIcon)
    //    return E_FAIL;

    //m_pSpark = dynamic_cast<CUI_SpriteAnim*>(FindPart(L"Part_PointStar_SpriteAnim"));
    //if (!m_pSpark)
    //    return E_FAIL;

    //// 아직 텍스트 안넣었음
    //m_pAmount = dynamic_cast<CUI_Text*>(FindPart(L"Part_PointStar_Amount"));

    //if (!m_pAmount)
    //    return E_FAIL;

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
