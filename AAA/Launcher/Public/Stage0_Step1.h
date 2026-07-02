#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CStage0_Step1 final : public CLevel
{
private:
	CStage0_Step1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CStage0_Step1() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();

public:
	static CStage0_Step1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END