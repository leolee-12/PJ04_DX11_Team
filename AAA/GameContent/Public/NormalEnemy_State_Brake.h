#pragma once
#include "Monster_State_Move.h"

NS_BEGIN(Client)
class CMonster;

class CNormalEnemy_State_Brake final : public CMonster_State_Move
{
protected:
    CNormalEnemy_State_Brake() = default;
    virtual ~CNormalEnemy_State_Brake() = default;

protected:
    virtual HRESULT                     Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE          Get_StateType() override;

    virtual void                        Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                        Update(_float fTimeDelta) override;
    virtual void                        Exit(MONSTER_STATE_TYPE eNextState) override;

protected:
    virtual void                        Apply_Movement(_float fTimeDelta) override;

private:
    // 감속 시간 - 이 시간동안 m_fSpeed -> 0 
    // "Brake" 클립 길이와 대략 맞춰야 함
    static constexpr _float             s_fBrakeTime = 0.5f;

public:
    static CNormalEnemy_State_Brake*    Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                        Free() override;
};

NS_END