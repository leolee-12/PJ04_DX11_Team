#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CPoppyBrosJr_State_WindUp final : public CMonster_State
{
protected:
    CPoppyBrosJr_State_WindUp() = default;
    virtual ~CPoppyBrosJr_State_WindUp() = default;

protected:
    virtual HRESULT                 Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE      Get_StateType() override;
    virtual void                    Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                    Update(_float fTimeDelta) override;
    virtual void                    Exit(MONSTER_STATE_TYPE eNextState) override;

public:
    static CPoppyBrosJr_State_WindUp* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);


protected:
    virtual void                    Free() override;
};

NS_END