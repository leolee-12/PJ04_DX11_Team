#pragma once
#include "Panel.h"

NS_BEGIN(AnimUITool)

class CPanel_Viewport final : public CPanel
{
private:
	CPanel_Viewport(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Viewport() = default;

public:
	_bool						Is_Hovered() const { return m_bHovered; }

public:
	virtual void				Render() override;
	void						Set_SRV(ID3D11ShaderResourceView* pSRV);

private:
	ID3D11ShaderResourceView*	m_pSRV = { nullptr };

private:
	static constexpr _float		m_fTargetAspect = { 1600.f / 900.f };
	_bool						m_bHovered = { false };

public:
	static CPanel_Viewport*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END