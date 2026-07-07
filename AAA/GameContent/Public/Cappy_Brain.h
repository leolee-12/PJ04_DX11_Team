#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)
class CMonster;

class CCappy_Brain final : public CMonster_Brain_FSM
{
protected:
	CCappy_Brain() = default;
	virtual ~CCappy_Brain() = default;

public:
	virtual void			Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

protected:
	virtual HRESULT			Initialize(CMonster* pOwner) override;

public:
	static CCappy_Brain*	Create(CMonster* pOwner);

private:
	_int					m_iAIType = { 0 };			
	_float					m_fTimer = { 0.f };

	static constexpr _float s_fIdleWait_Fix = { 2.5f }; 
	static constexpr _float s_fIdleWait_Chase = { 0.f };  
	static constexpr _float s_fChaseTime = { 2.0f };

protected:
	virtual void			Free() override;
};

NS_END