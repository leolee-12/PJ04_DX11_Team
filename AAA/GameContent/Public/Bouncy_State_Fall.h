#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CBouncy_State_Fall final : public CMonster_State
{
protected:
    CBouncy_State_Fall() = default;
    virtual ~CBouncy_State_Fall() = default;

protected:
    virtual HRESULT Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE Get_StateType() override;
    virtual void Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Exit(MONSTER_STATE_TYPE eNextState) override;

private:
    static constexpr _float s_fFallBlend = { 0.2f };

public:
    static CBouncy_State_Fall* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void Free() override;
};

NS_END