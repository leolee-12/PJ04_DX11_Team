#pragma once

#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CBrontoBurt_State_Return final : public CMonster_State
{
protected:
    CBrontoBurt_State_Return() = default;
    virtual ~CBrontoBurt_State_Return() = default;

protected:
    virtual HRESULT             Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE  Get_StateType() override;
    virtual void                Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                Update(_float fTimeDelta) override;
    virtual void                Exit(MONSTER_STATE_TYPE eNextState) override;

private:
    static constexpr _float     s_fReturnSpeed = { 4.f };

public:
    static CBrontoBurt_State_Return* Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                Free() override;
};

NS_END