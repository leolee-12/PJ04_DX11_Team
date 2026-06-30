#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(MapTool)

class CLoader;

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(TOOL_LEVEL eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	TOOL_LEVEL	m_eNextLevelID = { TOOL_LEVEL::END };
	CLoader*	m_pLoader = { nullptr };
	_bool		m_bLoadFailureHandled = { false };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TOOL_LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END
