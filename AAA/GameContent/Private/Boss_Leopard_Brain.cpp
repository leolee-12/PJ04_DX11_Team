#include "Boss_Leopard_Brain.h"
#include "Boss_Leopard.h"
#include "BT.h"
#include "Monster_Movement.h"

CBTNode* CBoss_Leopard_Brain::Build_PhaseTree(_int iPhase)
{
    UNREFERENCED_PARAMETER(iPhase);

    const _float fTurnSpeedDeg = 240.f;
    const _float fFacingDot = 0.96f;
    const _float fSpd = 1.5f;

    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };
    auto OneShot = [&](const string& c) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, false, 0.f, 1.f });
        };
    auto HoldLoop = [&](const string& c, _float fHold) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, true, fHold, 1.f });
        };

    // 커비를 향해 부드럽게 회전 (다 돌면 SUCCESS)
    auto bTurn = make_shared<bool>(false);
    auto* pTurnToTarget = CBTAction::Create(
        [this, bTurn, fTurnSpeedDeg, fFacingDot](CBlackboard* pBB, _float dt) -> BT_STATUS {
            if (!*bTurn) { m_pOwner->Get_BodyAnimator()->Play("Wait", true, false); *bTurn = true; }

            _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
            _vector vT = XMLoadFloat3(&vDir);
            if (XMVector3Equal(vT, XMVectorZero())) { *bTurn = false; return BT_STATUS::SUCCESS; }

            CTransform* pTf = m_pOwner->Get_Transform();
            _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
            _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vT));
            if (fDot >= fFacingDot) { *bTurn = false; return BT_STATUS::SUCCESS; }

            _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vT)
                - XMVectorGetX(vLook) * XMVectorGetZ(vT);
            _float fYaw = atan2f(fCross, fDot);
            _float fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
            _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
            pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
            return BT_STATUS::RUNNING;
        },
        [bTurn]() { *bTurn = false; });

    // 손톱 던지기: ThrowStart(5개 소환) -> Throw(순차 발사)
    auto MakeThrow = [&]() -> CBTNode* {
        return CBTSequence::Create({
            OneShot("ThrowStart"),
            OneShot("Throw"),
            });
        };

    // 인접 기둥으로 껑충 (CCT off 상태 -> 트랜스폼 직접 포물선)
    auto MakePillarJump = [&](bool bAdvance) -> CBTNode* {
        struct ARC { _float3 vStart{}, vEnd{}; _float t = 0.f; bool started = false; };
        auto arc = make_shared<ARC>();
        const _float fDur = 1.2f;
        const _float fHeight = 12.f;

        return CBTAction::Create(
            [this, arc, fDur, fHeight, fSpd, bAdvance](CBlackboard*, _float dt) -> BT_STATUS {
                auto* pLeo = static_cast<CBoss_Leopard*>(m_pOwner);
                CTransform* pTf = m_pOwner->Get_Transform();
                auto* mv = m_pOwner->Get_Movement();

                if (!arc->started) {
                    pLeo->Enter_PillarMode();
                    XMStoreFloat3(&arc->vStart, pTf->Get_State(STATE::POSITION));  // 현재 위치(첫 마운트=지상)
                    if (bAdvance) pLeo->Advance_ToAdjacentPillar();                // 첫 마운트는 그대로 FL
                    arc->vEnd = pLeo->Get_CurPillarPos();
                    arc->t = 0.f; arc->started = true;
                    mv->Face_Instant(XMLoadFloat3(&arc->vEnd));
                    m_pOwner->Get_BodyAnimator()->Play("PillarJump", false, true, 0.1f, fSpd);
                }
                arc->t += dt;
                _float u = arc->t / fDur; if (u > 1.f) u = 1.f;
                _vector vPos = XMVectorLerp(XMLoadFloat3(&arc->vStart), XMLoadFloat3(&arc->vEnd), u);
                _float  fHop = 4.f * fHeight * u * (1.f - u);
                vPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + fHop);
                pTf->Set_State(STATE::POSITION, vPos);
                mv->Sync_To_Controller();

                if (u >= 1.f) {
                    pTf->Set_State(STATE::POSITION, XMLoadFloat3(&arc->vEnd));
                    mv->Sync_To_Controller();
                    mv->Face_Instant(XMLoadFloat3(&m_pOwner->Get_BlackBoard().vTargetPos));
                    arc->started = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [arc]() { arc->started = false; });
        };

    auto bMounted = make_shared<bool>(false);
    auto* pMountFirst = CBTSelector::Create({
        // 이미 올라탔으면 즉시 통과
        CBTSequence::Create({
            CBTCondition::Create([bMounted](CBlackboard*) { return *bMounted; }),
            CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
            }),
            // 첫 사이클: 지상 -> FL 기둥 도약 후 latch
            CBTSequence::Create({
                MakePillarJump(false),
                CBTAction::Create([bMounted](CBlackboard*, _float) { *bMounted = true; return BT_STATUS::SUCCESS; }),
                }),
        });

    // 2번 던질 때마다 한 번 기둥 점프 (헤더 수정 없이 카운터는 shared_ptr로)
    auto iThrowCnt = make_shared<int>(0);
    auto* pMaybeJump = CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([iThrowCnt](CBlackboard*) { return (++(*iThrowCnt) % 2) == 0; }),
            MakePillarJump(true),               // 인접 기둥으로 이동
            }),
        CBTAction::Create([](CBlackboard*, _float) { return BT_STATUS::SUCCESS; }),
        });

    // 루트 Sequence는 SUCCESS 시 자동 리셋 -> 매 사이클 반복
    return CBTSequence::Create({
        pMountFirst,
        pTurnToTarget,
        MakeThrow(),
        HoldLoop("Wait", 0.6f),
        pMaybeJump,
        });
}

CBoss_Leopard_Brain* CBoss_Leopard_Brain::Create(CMonster* pOwner)
{
    CBoss_Leopard_Brain* pInstance = new CBoss_Leopard_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Leopard_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Leopard_Brain::Free() { __super::Free(); }