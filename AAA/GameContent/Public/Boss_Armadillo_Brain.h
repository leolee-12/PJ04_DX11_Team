#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Armadillo_Brain final : public CBoss_Brain
{
private:
    CBoss_Armadillo_Brain() = default;
    virtual ~CBoss_Armadillo_Brain() = default;

protected:
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    _bool m_bOpeningDone = { false };

    static constexpr _float SPD = 1.5f, TURN_DEG = 120.f, WALK_SPEED = 4.f;
    static constexpr _float CATCH_MOVE_SPEED = 8.f, ROLL_SPEED = 14.f, SOLO_ROLL_SPEED = 30.f;
    static constexpr _float WALL_PROBE = 3.5f, REST_TIME = 0.8f, GROGGY_TIME = 5.f;

private:
    CBTNode* Make_Rest();
    CBTNode* Make_PartnerAnim(const _char* szClip, _bool bLoop);
    CBTNode* Make_PartnerSpinHit(_bool b);
    CBTNode* Make_TwinDance();
    CBTNode* Make_Roll();
    CBTNode* Make_PartnerThrow();
    CBTNode* Make_CatchOnce(shared_ptr<bool> pThrown);
    CBTNode* Make_Catch();

public:
    static CBoss_Armadillo_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END