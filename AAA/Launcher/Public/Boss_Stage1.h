#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CBoss_Stage1 final : public CLevel
{
private:
	CBoss_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBoss_Stage1() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();

public:
	static CBoss_Stage1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END