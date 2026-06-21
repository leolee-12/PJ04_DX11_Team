#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Attack : public CMonster_State
{
protected:
	CMonster_State_Attack() = default;
	virtual ~CMonster_State_Attack() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter() override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CMonster_State_Attack*	Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
	_float3							m_vLungeDir = {}; // 공격 시 돌진할 방향 벡터

protected:
	virtual void					Free() override;
};

NS_END