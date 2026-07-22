#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CGigatzo_State_Attack final : public CMonster_State
{
protected:
    CGigatzo_State_Attack() = default;
    virtual ~CGigatzo_State_Attack() = default;

protected:
    virtual HRESULT                 Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE      Get_StateType() override;
    virtual void                    Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                    Update(_float fTimeDelta) override;
    virtual void                    Exit(MONSTER_STATE_TYPE eNextState) override;

public:
    static CGigatzo_State_Attack*   Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                    Free() override;
};

NS_END