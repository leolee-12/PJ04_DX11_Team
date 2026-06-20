#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"
#include "Animator.h"

NS_BEGIN(Client)
using ANI_PLAY_INFO = CAnimator::ANI_PLAY_INFO;

class CMonster;

class CMonster_State abstract : public CBase
{
protected:
	CMonster_State();
	virtual ~CMonster_State() = default;

public:
	virtual MONSTER_STATE_TYPE	Get_StateType() = 0;
	_bool						Is_Interruptible() const { return m_bIsInterruptible; }		// 기본 잠금

protected:
	virtual HRESULT				Initialize();
	virtual HRESULT				Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed = 0.f);

public:
	virtual void				Enter(CMonster* pMonster);

	virtual void				Update(CMonster* pMonster, _float fTimeDelta);

	virtual void				Exit(CMonster* pMonster);

protected:
	_bool						m_bIsInterruptible = { false };
	_float						m_fSpeed = { 0.f };		// 상태 동안 이동 적용할 속도

	_float						m_fTimer = { 0.f };
	_float						m_fMaxTime = { 0.f };

	ANI_PLAY_INFO				m_PlayInfo{};

protected:
	virtual void				Free() override;
};

NS_END
