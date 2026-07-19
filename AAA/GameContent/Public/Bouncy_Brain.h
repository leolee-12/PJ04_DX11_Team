#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CBouncy_Brain final : public CMonster_Brain_FSM
{
protected:
    CBouncy_Brain();
    virtual ~CBouncy_Brain() = default;

protected:
    virtual HRESULT         Initialize(CMonster* pOwner) override;
    virtual void            Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static CBouncy_Brain*   Create(CMonster* pOwner);

protected:
    virtual void            Free() override;
};

NS_END