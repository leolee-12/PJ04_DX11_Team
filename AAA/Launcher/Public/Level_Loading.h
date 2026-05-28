#pragma once

#include "Launcher_Defines.h"
#include "Level.h"


/* 로딩레벨에 필요한 객체(배경, 로딩바, 로딩텍스트)들을 생성한다. */
/* 로딩레벨을 갱신하여 화면에 보여준다. */
/* 다음 레벨을 위한 자원을 준비한다. */

NS_BEGIN(Client)

class CLoader;

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	LEVEL			m_eNextLevelID = { LEVEL::END };
	CLoader*		m_pLoader = { nullptr };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Load_LoadingLevel();

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END