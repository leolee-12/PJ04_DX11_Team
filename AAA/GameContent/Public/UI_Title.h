#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"
#include "UIAnimatorCom.h"

NS_BEGIN(Client)
class CUI_Image;
class CUI_SpriteAnim;
class CUI_Effect;
class CUI_Text;

class CUI_Title final : public CUIContainerObject, public IUIAnimatorOwner
{
	GENERATED_BODY(CUI_Title)

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_Title";

private:
	CUI_Title(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Title(const CUI_Title& Prototype);
	virtual ~CUI_Title() = default;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(void* pArg) override;
	virtual void						Priority_Update(_float fTimeDelta) override;
	virtual void						Update(_float fTimeDelta) override;
	virtual void						Late_Update(_float fTimeDelta) override;
	virtual void						Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual CUIAnimatorCom*				Get_UIAnimatorCom() override { return m_pUIAnimatorCom; }
	virtual const CUIAnimatorCom*		Get_UIAnimatorCom() const override { return m_pUIAnimatorCom; }

protected:
	virtual HRESULT						Ready_Events() override;
	virtual void						On_Deserialized() override;
	virtual void						On_UIPartsChanged() override;

private:
	HRESULT								Cache_Parts();
	void								Bind_UIAnimator();

	// юс╫ц╥н
	CUI_Image*							m_pIcon = { nullptr };
	CUI_Effect*							m_pEffect = { nullptr };
	CUIAnimatorCom*						 m_pUIAnimatorCom = { nullptr };

public:
	static CUI_Title*					Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject*						Clone(void* pArg) override;
	
protected:
	virtual void						Free() override;
};

NS_END
