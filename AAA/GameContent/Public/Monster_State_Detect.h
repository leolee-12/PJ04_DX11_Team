#pragma once
#include "Monster_StateT.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Detect final : public CMonster_StateT<CMonster>
{
private:
	CMonster_State_Detect() = default;
	virtual ~CMonster_State_Detect() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo, _float fSpped = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					On_Enter(CMonster* pMonster) override;
	virtual void					On_Update(CMonster* pMonster, _float fTimeDelta) override;
	virtual void					On_Exit(CMonster* pMonster) override;

public:
	static CMonster_State_Detect* Create(const ANI_PLAY_INFO& tInfo, _float fSpped = 0.f);

protected:
	virtual void					Free() override;
};

NS_END