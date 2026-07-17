#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CDekabu_State_Alert final : public CMonster_State
{
protected:
    CDekabu_State_Alert() = default;
    virtual ~CDekabu_State_Alert() = default;

protected:
    virtual HRESULT             Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE  Get_StateType() override;
    virtual void                Enter(MONSTER_STATE_TYPE ePrevState
        = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                Update(_float fTimeDelta) override;
    virtual void                Exit(MONSTER_STATE_TYPE eNextState) override;

public:
    static CDekabu_State_Alert* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                Free() override;
};

NS_END
