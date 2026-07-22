#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CNoddy_State_Find final : public CMonster_State
{
protected:
    CNoddy_State_Find() = default;
    virtual ~CNoddy_State_Find() = default;

protected:
    virtual HRESULT Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE Get_StateType() override;
    virtual void Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Exit(MONSTER_STATE_TYPE eNextState) override;

public:
    static CNoddy_State_Find* Create(const ANI_PLAY_INFO& tInfo, _float fSpeed = 0.f);

protected:
    virtual void Free() override;
};

NS_END