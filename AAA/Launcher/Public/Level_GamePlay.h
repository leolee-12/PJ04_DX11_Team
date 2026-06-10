#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLumia;

class CLevel_GamePlay final : public CLevel
{
private:
	CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GamePlay() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	enum class EEndingPhase { NONE, SLOW, HOLD, RECOVER };

	EEndingPhase m_eEndingPhase = { EEndingPhase::NONE };
	_float       m_fEndingTimer = { 0.f };

	static constexpr _float SLOWMO_RAMP_TIME = 0.4f;   
	static constexpr _float SLOWMO_TARGET = 0.2f;
	static constexpr _float HOLD_TIME = 1.5f;   
	static constexpr _float RECOVER_RAMP_TIME = 0.6f;

private:
	virtual HRESULT Ready_Events() override;
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	void Key_Input();
	void Update_EndingSequence(_float fRawDelta);
	void Start_Ending(CGameObject* pBoss);


public:
	static CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END