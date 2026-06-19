#pragma once
#include "Monster_StateT.h"

NS_BEGIN(Client)
class CBladeKnight;

class CBladeKnight_State_Fall final : public CMonster_StateT<CBladeKnight>
{
private:
	CBladeKnight_State_Fall() = default;
	virtual ~CBladeKnight_State_Fall() = default;

private:
	virtual HRESULT						Initialize() override;

public:
	virtual MONSTER_STATE_TYPE			Get_StateType() override;

protected:
	virtual void						On_Enter(CBladeKnight* pBladeKnight) override;
	virtual void						On_Update(CBladeKnight* pBladeKnight, _float fTimeDelta) override;
	virtual void						On_Exit(CBladeKnight* pBladeKnight) override;

public:
	static CBladeKnight_State_Fall*		Create();

protected:
	virtual void						Free() override;
};

NS_END