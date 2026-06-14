#pragma once
#include "AnimUITool_Defines.h"
#include "Panel.h"

NS_BEGIN(AnimUITool)

class CPanel_Palette final : public CPanel
{
private:
	CPanel_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Palette() = default;

public:
	virtual void			Render() override;

private:
	_uint					m_iSpawnCounter = { 0 };

public:
	static CPanel_Palette*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void			Free() override;
};

NS_END