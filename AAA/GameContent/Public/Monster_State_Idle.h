#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Idle : public CMonster_State
{
protected:
	CMonster_State_Idle() = default;
	virtual ~CMonster_State_Idle() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CMonster_State_Idle*		Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

private:
	static	constexpr _float		s_fFaceDeadZoneXZ = { 0.5f };

protected:
	virtual void					Free() override;
};

NS_END