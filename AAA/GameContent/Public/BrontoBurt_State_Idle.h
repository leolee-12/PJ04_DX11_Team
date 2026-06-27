#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CBrontoBurt_State_Idle final : public CMonster_State
{
protected:
    CBrontoBurt_State_Idle() = default;
    virtual ~CBrontoBurt_State_Idle() = default;

protected:
    virtual HRESULT                 Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE      Get_StateType() override;
    virtual void                    Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                    Update(_float fTimeDelta) override;
    virtual void                    Exit(MONSTER_STATE_TYPE eNextState) override;

public:
    static CBrontoBurt_State_Idle*  Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                    Free() override;
};

NS_END