#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"
#include "Animator.h"

NS_BEGIN(Client)
using ANI_PLAY_INFO = CAnimator::ANI_PLAY_INFO;

class CMonster;
class CMonster_Movement;

class CMonster_State abstract : public CBase
{
protected:
	CMonster_State();
	virtual ~CMonster_State() = default;

public:
	virtual MONSTER_STATE_TYPE	Get_StateType() = 0;
	MONSTER_STATE_TYPE			Get_NextState() const { return m_eNextState; }										// 명시적으로 다음 상태 지정이 필요할 경우 사용
	void						Set_NextState(MONSTER_STATE_TYPE eState) { m_eNextState = eState; }					// State 등록 이후 외부 주입 필요한 경우

	_bool						Is_Interruptible() const { return m_bIsInterruptible; }		// 기본 잠금
	_bool						Is_UseMoveWindow() const { return m_bUseMoveWindow; }		// 이동 상태 : false (연속적움직임) + true (AnimEvent Range  동안만 움직임)

protected:
	virtual HRESULT				Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed = 0.f);

public:
	virtual void				Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) = 0;

	virtual void				Update(_float fTimeDelta) = 0;

	virtual void				Exit(MONSTER_STATE_TYPE eNextState) = 0;

public:
	void						Set_Owner(CMonster* pOwner);

protected:
	CMonster*					m_pOwner = { nullptr };
	CAnimator*					m_pAnimator = { nullptr };
	CMonster_Movement*			m_pMovement = { nullptr };

	ANI_PLAY_INFO				m_PlayInfo{};

	MONSTER_STATE_TYPE			m_eNextState = { MONSTER_STATE_TYPE::IDLE };

	_bool						m_bIsInterruptible = { false };		// 중간에 가로챌 수 있는 상태인지
	_float						m_fSpeed = { 0.f };					// 상태 동안 이동 적용할 속도

	_float						m_fTimer = { 0.f };
	_float						m_fMaxTime = { 0.f };

	_bool						m_bUseMoveWindow = { false };

	_float3						m_MoveDir = {};

protected:
	virtual void				Free() override;
};

NS_END
