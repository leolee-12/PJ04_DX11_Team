#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)
class CUI_Image;
class CUI_SpriteAnim;
class CUI_Effect;
class CUI_Text;

class CLIENT_DLL CUI_PointStar final : public CUIContainerObject
{
	GENERATED_BODY(CUI_PointStar)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_PointStar";

private:
	CUI_PointStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_PointStar(const CUI_PointStar& Prototype);
	virtual ~CUI_PointStar() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual void			Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

protected:
	virtual HRESULT			Ready_Events() override;
	virtual void			On_Deserialized() override;
	virtual void			On_UIPartsChanged() override;

private:
	HRESULT					Cache_Parts();
	void					Play_PointStarBounce();
	void					On_PointStarGained(const KIRBY_POINTSTAR_GAINED_DESC* pDesc);
	void					Refresh_AmountText();

private:
	CUI_Image*				m_pIcon = { nullptr };
	CUI_SpriteAnim*			m_pSpark = { nullptr };
	CUI_Effect*				m_pEffect = { nullptr };
	CUI_Text*				m_pAmount = { nullptr };
	
	_uint					m_iStarCount = { 0 };
	_float					m_fVisibleDuration = { 3.0f };		// ¸¶Áö¸· È¹µæ ÈÄ À¯Áö ½Ã°£
	_float					m_fVisibleElapsed = { 0.f };


public:
	static CUI_PointStar*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject*			Clone(void* pArg) override;

protected:
	virtual void			Free() override;

};

NS_END
