#pragma once
#include "Panel.h"

NS_BEGIN(MapTool)

class CPanel_Palette final : public CPanel
{
private:
	CPanel_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Palette() = default;

public:
	virtual void				Render() override;

private:
	static constexpr _uint		BUF_SIZE = { 64 };
	_wstring					m_strPendingTag = {};
	_char						m_szNameBuf[BUF_SIZE] = {};
	_bool						m_bOpenNamePopup = { false };

public:
	static CPanel_Palette*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
