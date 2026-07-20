#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CNoddy_Brain final : public CMonster_Brain_FSM
{
protected:
    CNoddy_Brain();
    virtual ~CNoddy_Brain() = default;

protected:
    virtual HRESULT         Initialize(CMonster* pOwner) override;
    virtual void            Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static CNoddy_Brain*    Create(CMonster* pOwner);

protected:
    virtual void            Free() override;
};

NS_END
