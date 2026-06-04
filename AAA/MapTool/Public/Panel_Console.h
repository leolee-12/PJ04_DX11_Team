#pragma once
#include "Panel.h"

NS_BEGIN(MapTool)

class CPanel_Console final : public CPanel
{
private:
	CPanel_Console(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Console() = default;

public:
	virtual void				Render() override;

private:
	_bool						m_bShowInfo = { true };
	_bool						m_bShowWarning = { true };
	_bool						m_bShowError = { true };

public:
	static CPanel_Console*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
