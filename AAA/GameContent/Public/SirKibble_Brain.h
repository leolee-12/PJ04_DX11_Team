#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CSirKibble_Brain final : public CMonster_Brain_FSM
{
protected:
    CSirKibble_Brain();
    virtual ~CSirKibble_Brain() = default;

protected:
    virtual HRESULT           Initialize(CMonster* pOwner) override;

public:
    static CSirKibble_Brain*  Create(CMonster* pOwner);

protected:
    virtual void              Free() override;
};

NS_END
