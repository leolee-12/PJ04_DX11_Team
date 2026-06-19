#pragma once

#include "Base.h"
#include "GameContent_Defines.h"
#include "Monster_BlackBoard.h"

NS_BEGIN(Client)

class CMonster;

class CMonster_State abstract : public CBase
{
protected:
	CMonster_State();
	virtual ~CMonster_State() = default;

public:
	virtual MONSTER_STATE_TYPE	Get_StateType() = 0;
	_bool						Is_Interruptible() const { return m_bIsInterruptible; }		// 기본 잠금

public:
	// 상태 진입 시 1회 호출
	// 이 상태에서 켜야 할 것들을 셋업 
	virtual void				Enter(CMonster* pMonster);

	// 매 프레임 호출.  이 상태의 행동을 실행한다.
	// - 이동 의도, 공격 판정 등 
	// 애니 완료 감지 시 BlackBoard에 신호만 넣는다.
	// 다음 상태를 직접 결정/전이 하지 않는다. - Brain이 결정
	virtual void				Update(CMonster* pMonster, _float fTimeDelta);

	// 상태 이탈 시 1회 호출
	// Enter/Update가 세팅한 것들을 되돌린다. 
	// 완료/인터럽트(피격 등) 어떤 경로로 나가도 호출되므로 
	// 떠날 때 반드시 꺼야하는 것의 정리를 여기서 보장
	virtual void				Exit(CMonster* pMonster);

protected:
	_bool						m_bIsInterruptible = { false };

protected:
	virtual void				Free() override;
};

NS_END
