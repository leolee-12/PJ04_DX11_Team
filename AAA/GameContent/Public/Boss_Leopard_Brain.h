#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Leopard_Brain final : public CBoss_Brain
{
private:
    CBoss_Leopard_Brain() = default;
    virtual ~CBoss_Leopard_Brain() = default;

protected:
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    static constexpr _float SPD = 1.5f, TURN_DEG = 240.f, FACE_DOT = 0.96f;

private:
    const _char* IdleClip() const;
    CBTNode* Make_Turn();
    CBTNode* Make_IdleHold(_float fHold);
    CBTNode* Make_Throw();
    CBTNode* Make_Arc(bool bAdvance, vector<string> airClips, _float fDur, _float fHeight);
    CBTNode* Make_AdjacentJump();
    CBTNode* Make_Remount();
    CBTNode* Make_MountFirst();
    CBTNode* Make_LeadIn();
    CBTNode* Make_Dash();
    CBTNode* Make_Groggy();
    CBTNode* Make_ChargeDash();
    CBTNode* Make_DropClaw();
    CBTNode* Make_AssaultSlash();
    CBTNode* Make_GroundPattern();
    CBTNode* Make_GroundPhase();

public:
    static CBoss_Leopard_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END