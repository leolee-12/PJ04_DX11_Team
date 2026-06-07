#pragma once
#include "Panel.h"

NS_BEGIN(Engine)
class CUIContainerObject;
class CUIPartObject;
NS_END

NS_BEGIN(AnimUITool)

class CPanel_UICanvas final : public CPanel
{
private:
	CPanel_UICanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_UICanvas() = default;

	struct UI_PART_BOUNDS
	{
		_float2					vCenter = {};
		_float2					vSize = {};
		_float					fZ = {};
		RENDERUIID				eRenderLayer = { RENDERUIID::MIDDLE };
	};

	struct UI_PICK_CANDIDATE
	{
		CUIContainerObject* pContainer = { nullptr };
		CUIPartObject* pPart = { nullptr };
		_wstring				strPartTag = {};
		UI_PART_BOUNDS			Bounds = {};
	};

public:
	_bool						Is_Hovered() const { return m_bHovered; }
	_bool						Is_CanvasHovered() const { return m_bHovered; }

public:
	virtual void				Render() override;

	void						Set_SRV(ID3D11ShaderResourceView* pSRV);
	void						Set_Aspect(_float fAspect) { m_fTargetAspect = fAspect; }
	void						Set_DesignSize(_float fWidth, _float fHeight);

	_bool						ScreenToDesignPos(const _float2& vScreenPos, _float2* pOutDesignPos) const;
	_bool						ScreenToUIPos(const _float2& vScreenPos, _float2* pOutUIPos) const;

private:
	void						Draw_Grid(_float fX, _float fY, _float fW, _float fH);

	void						Handle_Selection();
	void						Draw_SelectedPart();
	void						Clear_UISelection();

	_bool						Build_PartBounds(CUIContainerObject* pContainer, CUIPartObject* pPart, UI_PART_BOUNDS* pOutBounds) const;
	_bool						Pick_TopmostPart(const _float2& vMouseUI, UI_PICK_CANDIDATE* pOutCandidate) const;

	_bool						Hit_SelectedHandle(const _float2& vMouseScreen, UI_DRAG_MODE* pOutMode) const;
	void						Begin_Drag(UI_DRAG_MODE eMode, const _float2& vMouseUI);
	void						Update_Drag(const _float2& vMouseUI);
	void						End_Drag();

	_bool						UIToScreenPos(const _float2& vUIPos, _float2* pOutScreenPos) const;
	_bool						UIToLocalDelta(const _float2& vUIDelta, _float2* pOutLocalDelta) const;

private:
	ID3D11ShaderResourceView* m_pSRV = { nullptr };

	_float						m_fTargetAspect = { 1600.f / 900.f };

	_float2						m_vCanvasMin = {};
	_float2						m_vCanvasSize = {};

	_float2						m_vMouseDesignPos = {};
	_float2						m_vMouseUIPos = {};
	_bool						m_bHasMouseCanvasPos = { false };

	_bool						m_bHovered = { false };

	UI_DRAG_MODE				m_eDragMode = { UI_DRAG_MODE::NONE };
	_float2						m_vDragStartMouseUI = {};
	_float2						m_vDragStartCenter = {};
	_float2						m_vDragStartSize = {};
	_float						m_fDragStartZ = { 1.f };

	_char						m_szSaveName[128] = {};
	_bool						m_bOpenSavePopup = { false };

public:
	static CPanel_UICanvas* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;

};

NS_END