#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CGigatzo_Brain final : public CMonster_Brain_FSM
{
protected:
	CGigatzo_Brain();
	virtual ~CGigatzo_Brain() = default;

protected:
    virtual HRESULT                 Initialize(CMonster* pOwner) override;
    virtual void                    Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static  CGigatzo_Brain*         Create(CMonster* pOwner);

private:
    _bool                           m_bArmed = { false };
    _float m_fFireTimer = { 0.f };

    static constexpr const _float   s_fFireInterval = { 0.85f };

protected:
    virtual void                    Free() override;

};

NS_END