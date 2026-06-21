#pragma once
#include "Monster_State_Attack.h"

NS_BEGIN(Client)
class CBladeKnight;

class CBladeKnight_State_TornadoAttack final : public CMonster_State_Attack
{
private:
	CBladeKnight_State_TornadoAttack() = default;
	virtual ~CBladeKnight_State_TornadoAttack() = default;

private:
	virtual HRESULT						Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE			Get_StateType() override;

protected:
	virtual void						Enter() override;
	virtual void						Update(_float fTimeDelta) override;
	virtual void						Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CBladeKnight_State_TornadoAttack* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

private:
	_float3								m_vLungeDir = {}; 

protected:
	virtual void						Free() override;
};

NS_END