#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CRabbitEnemy_State_Jump final : public CMonster_State
{
protected:
    CRabbitEnemy_State_Jump() = default;
    virtual ~CRabbitEnemy_State_Jump() = default;

protected:
    virtual HRESULT Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE Get_StateType() override;
    virtual void Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Exit(MONSTER_STATE_TYPE eNextState) override;

private:
    void Begin_Jump();

private:
    _bool  m_bJumped = { false };

public:
    static CRabbitEnemy_State_Jump* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void Free() override;
};

NS_END
