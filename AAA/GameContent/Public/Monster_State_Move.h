#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster_State_Move abstract : public CMonster_State
{
protected:
	CMonster_State_Move() = default;
	virtual ~CMonster_State_Move() = default;

protected:
	virtual HRESULT		Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual void		Enter() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
	// 파생 클래스에서 자기 이동 로직 구현 (방향만 적용)
	virtual void		Apply_Movement(_float fTimeDelta) = 0;

protected:
	virtual void		Free() override;
};

NS_END