#pragma once
#include "Monster_StateT.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Captured final : public CMonster_StateT<CMonster>
{
private:
	CMonster_State_Captured() = default;
	virtual ~CMonster_State_Captured() = default;

protected:
	virtual HRESULT					Initialize(const ANI_PLAY_INFO& tInfo, _float fSpped = 0.f) override;

public:
	virtual MONSTER_STATE_TYPE		Get_StateType() override;
	virtual void					On_Enter(CMonster* pMonster) override;
	virtual void					On_Update(CMonster* pMonster, _float fTimeDelta) override;
	virtual void					On_Exit(CMonster* pMonster) override;

private:
	_float							m_fPullSpeed = { 0.f };
	_float3							m_vBaseScale = {};
	_float							m_fScaleRatio = { 1.f };

	static constexpr _float			s_fPullInitSpeed =  0.f;
	static constexpr _float			s_fPullAccel = 40.f;
	static constexpr _float			s_fMinScaleRatio = 0.15f;
	static constexpr _float			s_fShrinkLerp = 2.f;

public:
	static CMonster_State_Captured* Create(const ANI_PLAY_INFO& tInfo, _float fSpped = 0.f);

protected:
	virtual void					Free() override;
};

NS_END