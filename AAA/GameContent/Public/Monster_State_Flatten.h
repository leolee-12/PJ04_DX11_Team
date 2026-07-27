#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Flatten : public CMonster_State
{
protected:
	CMonster_State_Flatten() = default;
	virtual ~CMonster_State_Flatten() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
	virtual void					Update(_float fTimeDelta) override;
	virtual void					Exit(MONSTER_STATE_TYPE eNextState) override;

public:
	static CMonster_State_Flatten* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

private:
	_float3							m_vBaseScale = {};

	static constexpr _float			s_fSquishDur = 0.02f;	// 납작해지는 시간
	static constexpr _float			s_fLifeTime = 0.25f;	// 소멸까지
	static constexpr _float			s_fFlatY = 0.12f;		// 최종 Y 배율
	static constexpr _float			s_fSpreadXZ = 1.25f;	

protected:
	virtual void					Free() override;
};

NS_END