#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CRangerEnemy_Brain final : public CMonster_Brain_FSM
{
protected:
    CRangerEnemy_Brain();
    virtual ~CRangerEnemy_Brain() = default;

protected:
    virtual HRESULT             Initialize(CMonster* pOwner) override;

public:
    static CRangerEnemy_Brain*  Create(CMonster* pOwner);

protected:
    virtual void                Free() override;
};

NS_END
