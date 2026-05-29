#pragma once
#include "Panel.h"

NS_BEGIN(AnimUITool)

class CPanel_Inspector final : public CPanel
{
private:
	CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Inspector() = default;

public:
	virtual void				Render() override;

public:
	static CPanel_Inspector*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END