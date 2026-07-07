#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CCappy_State_HatLose final : public CMonster_State
{
protected:
    CCappy_State_HatLose() = default;
    virtual ~CCappy_State_HatLose() = default;

protected:
    virtual HRESULT             Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE  Get_StateType() override;
    virtual void                Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void				Update(_float fTimeDelta) {};
    virtual void				Exit(MONSTER_STATE_TYPE eNextState) {};

public:
    static CCappy_State_HatLose* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                Free() override;
};

NS_END