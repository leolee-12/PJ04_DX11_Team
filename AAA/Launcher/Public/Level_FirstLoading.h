#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CTitleUI;
class CLoader;

class CLevel_FirstLoading final : public CLevel
{
private:
	CLevel_FirstLoading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_FirstLoading() = default;

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CLoader* m_pLoader = { nullptr };
	LEVEL m_eNextLevelID = {};

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Proto();
	HRESULT Ready_MyResources();

public:
	static CLevel_FirstLoading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END