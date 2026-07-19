#include "Boss_Metaknight_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Monster_Movement.h"

namespace
{
    inline _float Saturate(_float f)
    {
        return f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
    }

    inline _float SmoothStep01(_float t)
    {
        t = Saturate(t);
        return t * t * (3.f - 2.f * t);
    }
}

CBTNode* CBoss_Metaknight_Brain::Build_PhaseTree(_int)
{
    return CBTSequence::Create({
        Make_Rest(),
        Make_StepApproach(),
        Make_DashIn(),
        Make_Rest(),
        // TODO: 붙은 뒤 근접 공격(Attack1~3 콤보) 이어붙일 자리
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_Rest() { return Loop("Wait", REST_TIME, SPD); }

CBTNode* CBoss_Metaknight_Brain::Make_Step()
{
    auto bOn = make_shared<bool>(false);

    return CBTAction::Create(
        [this, bOn](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bOn)
            {
                Anim()->Play("FrontMove", false, true, 0.1f, SPD);
                *bOn = true;
            }

            if (pBB->Get<_float>("DistToTarget", FLT_MAX) <= STEP_KEEP_DIST)
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }

            _vector vDir = Dir_ToTargetXZ();
            RotateYawTo(vDir, TURN_DEG, dt);

            m_pOwner->Get_Movement()->Set_WindowMoveSpeed(STEP_SPEED, Anim()->Get_Progress());
            m_pOwner->Add_MoveDir(m_pOwner->Get_Transform()->Get_State(STATE::LOOK));

            if (Anim()->Is_Finished())
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn] { *bOn = false; });
}

CBTNode* CBoss_Metaknight_Brain::Make_StepApproach()
{
    return CBTSequence::Create({
        Make_Step(), Loop("Wait", STEP_PAUSE, SPD),
        Make_Step(), Loop("Wait", STEP_PAUSE, SPD),
        Make_Step(),
        });
}

CBTNode* CBoss_Metaknight_Brain::Make_DashIn()
{
    auto bOn = make_shared<bool>(false);
    auto fElapsed = make_shared<_float>(0.f);
    auto vLockPos = make_shared<_float3>();
    auto vLockDir = make_shared<_float3>();

    return CBTAction::Create(
        [this, bOn, fElapsed, vLockPos, vLockDir](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bOn)
            {
                _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
                _vector vTarget = XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos);

                _vector vDir = XMVectorSetY(vTarget - vSelf, 0.f);
                const _float fDist = XMVectorGetX(XMVector3Length(vDir));

                if (fDist <= DASH_STOP_DIST + DASH_ARRIVE_DIST)
                    return BT_STATUS::SUCCESS;

                vDir = XMVector3Normalize(vDir);

                XMStoreFloat3(vLockPos.get(), vTarget - vDir * DASH_STOP_DIST);
                XMStoreFloat3(vLockDir.get(), vDir);

                m_pOwner->Get_Movement()->Face_Instant(vTarget);

                Anim()->Play("Dash", true, true, 0.1f, SPD);
                *fElapsed = 0.f;
                *bOn = true;
            }

            *fElapsed += dt;

            m_pOwner->Get_Movement()->Set_MoveSpeed(DASH_SPEED);
            m_pOwner->Add_MoveDir(XMLoadFloat3(vLockDir.get()));

            _vector vSelf = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
            _vector vToPoint = XMVectorSetY(XMLoadFloat3(vLockPos.get()) - vSelf, 0.f);

            const _float fRemain = XMVectorGetX(XMVector3Length(vToPoint));
            const _bool  bPassed = XMVectorGetX(XMVector3Dot(vToPoint, XMLoadFloat3(vLockDir.get()))) < 0.f;

            if (fRemain <= DASH_ARRIVE_DIST || bPassed || *fElapsed >= DASH_TIMEOUT)
            {
                *bOn = false;
                return BT_STATUS::SUCCESS;
            }
            return BT_STATUS::RUNNING;
        },
        [bOn, fElapsed]
        {
            *bOn = false;
            *fElapsed = 0.f;
        });
}

CBoss_Metaknight_Brain* CBoss_Metaknight_Brain::Create(CMonster* pOwner)
{
    CBoss_Metaknight_Brain* pInstance = new CBoss_Metaknight_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner))) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_Brain::Free() { __super::Free(); }