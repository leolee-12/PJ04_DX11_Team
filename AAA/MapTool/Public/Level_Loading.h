#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLoader;

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(EDIT_LEVEL eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	EDIT_LEVEL			m_eNextLevelID = { EDIT_LEVEL::END };
	CLoader* m_pLoader = { nullptr };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, EDIT_LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END