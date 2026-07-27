#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CTown_Step3 final : public CLevel
{
private:
	CTown_Step3(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CTown_Step3() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_Lights();

public:
	static CTown_Step3* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END