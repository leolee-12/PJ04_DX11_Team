#pragma once

#include "Panel.h"

NS_BEGIN(MapTool)

class CPanel_Profiler final : public CPanel
{
private:
	CPanel_Profiler(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Profiler() = default;

public:
	virtual void Render() override;

public:
	static CPanel_Profiler* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void Free() override;
};

NS_END
