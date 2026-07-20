#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CArena final : public CLevel
{
private:
	CArena(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CArena() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_Lights();

public:
	static CArena* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END