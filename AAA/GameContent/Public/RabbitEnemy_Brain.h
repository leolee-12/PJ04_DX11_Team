#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CRabbitEnemy_Brain final : public CMonster_Brain_FSM
{
protected:
    CRabbitEnemy_Brain();
    virtual ~CRabbitEnemy_Brain() = default;

protected:
    virtual HRESULT         Initialize(CMonster* pOwner) override;
    virtual void            Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static CRabbitEnemy_Brain*  Create(CMonster* pOwner);

protected:
    virtual void            Free() override;
};

NS_END
