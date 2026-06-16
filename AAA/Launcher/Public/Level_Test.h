#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CTitleUI;

class CLevel_Test final : public CLevel
{
private:
	CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Test() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool	m_bTestLevelChange = { false };

private:
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();

public:
	static CLevel_Test* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END