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
	_bool						Is_Interruptible() const { return m_bIsInterruptible; }		// 기본 잠금

protected:
	virtual HRESULT				Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed = 0.f);

public:
	virtual void				Enter() = 0;

	virtual void				Update(_float fTimeDelta) = 0;

	virtual void				Exit(MONSTER_STATE_TYPE eNextState) = 0;

public:
	void						Set_Owner(CMonster* pOwner);

protected:
	CMonster*					m_pOwner = { nullptr };
	CAnimator*					m_pAnimator = { nullptr };
	CMonster_Movement*			m_pMovement = { nullptr };

	ANI_PLAY_INFO				m_PlayInfo{};

	_bool						m_bIsInterruptible = { false };		// 중간에 가로챌 수 있는 상태인지
	_float						m_fSpeed = { 0.f };					// 상태 동안 이동 적용할 속도

	_float						m_fTimer = { 0.f };
	_float						m_fMaxTime = { 0.f };

protected:
	virtual void				Free() override;
};

NS_END
