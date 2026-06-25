#pragma once
#include "Monster_State_Move.h"

NS_BEGIN(Client)
class CMonster;

class CNormalEnemy_State_Chase	final : public CMonster_State_Move
{
protected:
    CNormalEnemy_State_Chase() = default;
    virtual ~CNormalEnemy_State_Chase() = default;

protected:
    virtual HRESULT                     Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE          Get_StateType() override;

    virtual void                        Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                        Update(_float fTimeDelta) override;

protected:
    virtual void                        Apply_Movement(_float fTimeDelta) override;

private:
    // 추적(재조준) 시간 
    static constexpr _float             s_fTrackTime = 1.25f;

    // 총 추적 시간
    static constexpr _float             s_fChaseTime = 3.0f;

public:
    static CNormalEnemy_State_Chase*    Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                        Free() override;
};

NS_END