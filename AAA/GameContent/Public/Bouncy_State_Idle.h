#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CBouncy_State_Idle final : public CMonster_State
{
protected:
	CBouncy_State_Idle() = default;
	virtual ~CBouncy_State_Idle() = default;

protected:
	virtual HRESULT Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE Get_StateType() override;
	virtual void Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Exit(MONSTER_STATE_TYPE eNextState) override;

private:
	void Begin_Sequence();

private:
	static constexpr _float s_fRestTime       = { 0.f };
	static constexpr _float s_fJumpStartBlend = { 0.2f };
	static constexpr _float s_fJumpSBlend     = { 0.2f };
	static constexpr _float s_fJumpBlend      = { 0.2f };

	_float m_fRestTimer       = { 0.f };
	_bool  m_bSequenceStarted = { false };
	_bool  m_bJumped          = { false };

public:
	static CBouncy_State_Idle* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	virtual void Free() override;
};

NS_END