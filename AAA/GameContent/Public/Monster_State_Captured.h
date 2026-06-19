#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster;

class CMonster_State_Captured final : public CMonster_State
{
private:
	CMonster_State_Captured();
	virtual ~CMonster_State_Captured() = default;

public:
	HRESULT							Initialize();

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(CMonster* pMonster) override;
	virtual void					Update(CMonster* pMonster, _float fTimeDelta) override;
	virtual void					Exit(CMonster* pMonster) override;

private:
	_float m_fPullSpeed = { 0.f };
	_float3 m_vBaseScale = {};    
	_float  m_fScaleRatio = { 1.f };

	static constexpr _float s_fPullInitSpeed = 0.f;    
	static constexpr _float s_fPullAccel = 40.f;  
	static constexpr _float s_fMinScaleRatio = 0.15f; 
	static constexpr _float s_fShrinkLerp = 2.f;

public:
	static CMonster_State_Captured* Create();

protected:
	virtual void					Free() override;
};

NS_END