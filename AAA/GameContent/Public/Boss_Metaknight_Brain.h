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
    virtual _int     Get_PhaseCount() const override { return 2; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    static constexpr _float SPD = CBoss_Metaknight::s_fDefaultAnimSpeed;
    static constexpr _float TURN_DEG = 360.f;

    static constexpr _float STEP_SPEED = 4.f;        // 사인 윈도우 피크 속도
    static constexpr _float STEP_PAUSE = 0.15f;       // 스텝 사이 멈춤

    static constexpr _float SIDE_SPEED = 5.f;
    static constexpr _float RADIUS_GAIN = 1.5f;

    static constexpr _float DASH_SPEED = 15.f;       // 돌진 속도 (고정)
    static constexpr _float DASH_STOP_DIST = 3.f;    // 락온 지점 = 커비 앞 이 거리
    static constexpr _float DASH_ARRIVE_DIST = 0.5f; // 도착 판정 반경
    static constexpr _float DASH_TIMEOUT = 2.5f;

    static constexpr _float COMBO_RANGE = 6.f;
    static constexpr _float ATK_LUNGE_SPEED = 10.f;

    static constexpr _float DODGE_DIST = 10.f;        

    static constexpr _float GIGA_FLY_H = 6.f;      
    static constexpr _float GIGA_FLY_SPEED = 25.f; 
    static constexpr _float GIGA_RISE_SPEED = 12.f;
    static constexpr _float GIGA_ARRIVE = 0.6f;

    static constexpr _float ROCK_RISE_HEIGHT = 10.f;
    static constexpr _float ROCK_RISE_SPEED = 2.5f;
    static constexpr _float ROCK_JUMP_SPEED = 1.f;

    static constexpr _float ROCK_TRACK_SPEED = 12.f;    
    static constexpr _float DIVEBOMB_RANGE = 4.f;
    static constexpr _float DIVEBOMB_FALL_SPEED = 20.f;
    static constexpr _float ROCK_TRACK_Y = 13.f;
    static constexpr _float TRACK_DESCEND_SPEED = 12.f;

    static constexpr _float BASE_SPEED = 6.f;          // Initialize에서 준 기본 이동속도

    static constexpr _float UC_BACK_SPEED = 5.f;       // Step 중 뒤로 빠지는 속도
    static constexpr _float UC_CHARGE_TIME = 1.5f;     // Charge 유지 시간
    static constexpr _float UC_RUSH_SPEED = 40.f;      // 돌진 속도
    static constexpr _float UC_RUSH_MAX_DIST = 20.f;   // 이만큼 달리면 실패 판정 -> 브레이크
    static constexpr _float UC_RUSH_TIMEOUT = 1.5f;     // 벽에 막혔을 때 안전장치
    static constexpr _float UC_BRAKE_DIST = 2.f;       // 발 끌면서 미끄러지는 거리
    static constexpr _float UC_BRAKE_TURN_DEG = 300.f; // 미끄러지며 커비 쪽으로 도는 속도

private:
    _int m_iLastCombo = { 0 };

private:
    // 움직임
    CBTNode* Make_Step();        
    CBTNode* Make_StepApproach();
    CBTNode* Make_DashIn();
    CBTNode* Make_SideStep(_bool bRight);
    CBTNode* Make_RandStep();
    CBTNode* Make_Dodge();

    // 기본공격
    CBTNode* Make_SwordHit(_bool bOn);
    CBTNode* Make_SwordCombo(_int iHits);
    CBTNode* Make_AttackLunge(const string& strClip);

    //콤보
    CBTNode* Make_ComboPick();

    // 기가문샷
    CBTNode* Make_GigaFly();
    CBTNode* Make_GigaMoonShot();

    //낙석패턴
    _bool    FlyNoClip(_fvector vGoal, _float fSpeed, _float dt, _float fArrive);
    void     RiseToward(_float fTargetY, _float dt);
    CBTNode* Make_RockFly();
    CBTNode* Make_RockDrop(); 

    // 돌진잡기/격돌
    CBTNode* Make_UpperCalibur();
    CBTNode* Make_UC_BackStep();
    CBTNode* Make_UC_Charge();
    CBTNode* Make_UC_Rush(shared_ptr<bool> bCaught, shared_ptr<_float3> vDir);
    CBTNode* Make_UC_Brake(shared_ptr<_float3> vDir);
    CBTNode* Make_UC_CatchSuccess();
    CBTNode* Make_UpperBranch();


    // 트리 조립 유틸
    CBTNode* Make_Optional(CBTNode* pCond, CBTNode* pBody);
    CBTNode* Make_UnlessInRange(CBTNode* pNode);

    // 트리 분기
    CBTNode* Make_DodgeBranch();
    CBTNode* Make_GigaBranch();
    CBTNode* Make_ComboBranch();
    CBTNode* Make_RockBranch();


public:
    static CBoss_Metaknight_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END