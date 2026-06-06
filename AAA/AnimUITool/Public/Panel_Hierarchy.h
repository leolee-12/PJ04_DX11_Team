#pragma once

#include "Panel.h"

NS_BEGIN(AnimUITool)

class CPanel_Hierarchy final : public CPanel
{
private:
	CPanel_Hierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Hierarchy() = default;

public:
	virtual void				Render() override;

public:
	static CPanel_Hierarchy*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END