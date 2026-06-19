#pragma once
#include "Monster_StateT.h"

NS_BEGIN(Client)
class CBladeKnight;

class CBladeKnight_State_Attack final : public CMonster_StateT<CBladeKnight>
{
private:
	CBladeKnight_State_Attack() = default;
	virtual ~CBladeKnight_State_Attack() = default;

private:
	virtual HRESULT						Initialize() override;

public:
	virtual MONSTER_STATE_TYPE			Get_StateType() override;

protected:
	virtual void						On_Enter(CBladeKnight* pBladeKnight) override;
	virtual void						On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta) override;
	virtual void						On_Exit(CBladeKnight* pBladeKnight) override;

public:
	static CBladeKnight_State_Attack*	Create();

private:
	_float3								m_vLungeDir = {}; // 공격 시 돌진할 방향 벡터

protected:
	virtual void						Free() override;
};

NS_END