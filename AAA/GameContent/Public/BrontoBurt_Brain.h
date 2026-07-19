#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CBrontoBurt_Brain final : public CMonster_Brain_FSM
{
protected:
    CBrontoBurt_Brain();
    virtual ~CBrontoBurt_Brain() = default;

protected:
    virtual HRESULT             Initialize(CMonster* pOwner) override;
    virtual void                Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static CBrontoBurt_Brain*   Create(CMonster* pOwner);

private:    
    _float                      m_fTimer = { 0.f };
    static constexpr _float     s_fFallRecoverTime = { 1.5f };

protected:
    virtual void                Free() override;
};

NS_END