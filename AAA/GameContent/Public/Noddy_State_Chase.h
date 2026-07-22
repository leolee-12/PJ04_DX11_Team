#pragma once
#include "Monster_State_Move.h"

NS_BEGIN(Client)

class CNoddy_State_Chase final : public CMonster_State_Move
{
protected:
    CNoddy_State_Chase() = default;
    virtual ~CNoddy_State_Chase() = default;

protected:
    virtual HRESULT Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE Get_StateType() override;
    virtual void Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void Update(_float fTimeDelta) override;

protected:
    virtual void Apply_Movement(_float fTimeDelta) override;

private:
    static constexpr _float s_fWalkTime = 2.5f;

public:
    static CNoddy_State_Chase* Create(const ANI_PLAY_INFO& tInfo, _float fSpeed);

protected:
    virtual void Free() override;
};

NS_END
