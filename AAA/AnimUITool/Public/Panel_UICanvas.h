#pragma once
#include "Panel.h"

NS_BEGIN(AnimUITool)

class CPanel_UICanvas final : public CPanel
{
private:
	CPanel_UICanvas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_UICanvas() = default;

public:
	_bool						Is_Hovered() const { return m_bHovered; }
	_bool						Is_CanvasHovered() const { return m_bHovered; }

public:
	virtual void				Render() override;

	void						Set_SRV(ID3D11ShaderResourceView* pSRV);
	void						Set_Aspect(_float fAspect) { m_fTargetAspect = fAspect; }
	void						Set_DesignSize(_float fWidth, _float fWeight);

	_bool						ScreenToDesignPos(const _float2& vScreenPos, _float2* pOutDesignPos) const;
	_bool						ScreenToUIPos(const _float2& vScreenPos, _float2* pOutUIPos) const;

private:
	void						Draw_Grid(_float fX, _float fY, _float fW, _float fH);

private:
	ID3D11ShaderResourceView*	m_pSRV = { nullptr };

	_float2                     m_vDesignSize = { 1600.f, 900.f };
	_float                      m_fTargetAspect = { 1600.f / 900.f };
	_float                      m_fGridStep = { 100.f };

	_float2                     m_vCanvasMin = {};
	_float2                     m_vCanvasSize = {};

	_float2                     m_vMouseDesignPos = {};
	_float2                     m_vMouseUIPos = {};
	_bool                       m_bHasMouseCanvasPos = { false };

	_bool                       m_bShowGrid = { true };
	_bool                       m_bHovered = { false };

public:
	static CPanel_UICanvas*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;

};

NS_END