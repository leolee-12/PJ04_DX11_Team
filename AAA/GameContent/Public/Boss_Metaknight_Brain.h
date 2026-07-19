#pragma once
#include "Boss_Brain.h"
#include "Boss_Metaknight.h"

NS_BEGIN(Client)

class CBoss_Metaknight_Brain final : public CBoss_Brain
{
private:
    CBoss_Metaknight_Brain() = default;
    virtual ~CBoss_Metaknight_Brain() = default;

protected:
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    static constexpr _float SPD = CBoss_Metaknight::s_fDefaultAnimSpeed;
    static constexpr _float TURN_DEG = 360.f;
    static constexpr _float REST_TIME = 0.8f;

    static constexpr _float STEP_SPEED = 4.f;        // 사인 윈도우 피크 속도
    static constexpr _float STEP_PAUSE = 0.3f;       // 스텝 사이 멈춤
    static constexpr _float STEP_KEEP_DIST = 8.f;

    static constexpr _float DASH_SPEED = 15.f;       // 돌진 속도 (고정)
    static constexpr _float DASH_STOP_DIST = 2.f;    // 락온 지점 = 커비 앞 이 거리
    static constexpr _float DASH_ARRIVE_DIST = 0.5f; // 도착 판정 반경
    static constexpr _float DASH_TIMEOUT = 2.5f;

    static constexpr _float COMBO_RANGE = 6.f;

private:
    // 움직임
    CBTNode* Make_Rest();
    CBTNode* Make_Step();        
    CBTNode* Make_StepApproach();
    CBTNode* Make_DashIn();

    // 기본공격
    CBTNode* Make_SwordHit(_bool bOn);
    CBTNode* Make_SwordCombo();

public:
    static CBoss_Metaknight_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END