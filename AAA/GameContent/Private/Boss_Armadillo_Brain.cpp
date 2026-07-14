#include "Boss_Armadillo_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Monster_Movement.h"
#include "MultiHitBoxPart.h"
#include "Boss_Armadillo.h"
#include "Boss_Armadillo_Body.h"

CBTNode* CBoss_Armadillo_Brain::Build_PhaseTree(_int iPhase)
{
    UNREFERENCED_PARAMETER(iPhase);

#ifdef _DEBUG
    m_bOpeningDone = true;
#endif

    // ---- 튜닝 상수 ----
    const _float fSpd = 1.5f;
    const _float fCatchRange = 8.f;      // 루프에서 잡기를 고르는 거리
    const _float fTurnSpeedDeg = 120.f;
    const _float fWalkSpeed = 4.f;      // 기본 이동 속도 (Initialize와 일치)
    const _float fRollSpeed = 14.f;     // 구르기 돌진 속도
    const _float fWallProbe = 3.5f;     // 전방 벽 감지 스윕 거리
    const _float fRestTime = 0.8f;     // 패턴 사이 숨 고르기
    const _float fGroggyTime = 5.f;

    // ---- 공용 헬퍼 ----
    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };
    auto OneShot = [&](const string& c) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, false, 0.f, fSpd });
        };
    auto HoldLoop = [&](const string& c, _float fHold) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, true, fHold, fSpd });
        };
    auto Rest = [&]() -> CBTNode* { return HoldLoop("Wait", fRestTime); };
    auto PlayFacing = [&](const string& c, _float fTurnDeg = 120.f, _float fSpeedMul = 1.f) -> CBTNode* {
        auto bP = make_shared<bool>(false);
        return CBTAction::Create(
            [this, bP, c, fSpd, fTurnDeg, fSpeedMul](CBlackboard* pBB, _float dt) -> BT_STATUS {
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bP) { pAnim->Play(c, false, true, 0.1f, fSpd * fSpeedMul); *bP = true; }

                _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                _vector vToTgt = XMLoadFloat3(&vDir);
                if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                    CTransform* pTf = m_pOwner->Get_Transform();
                    _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                    _float  fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                    _float  fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                        - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                    _float  fYaw = atan2f(fCross, fDot);
                    _float  fStep = XMConvertToRadians(fTurnDeg) * dt;
                    _float  fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                    pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                }
                if (pAnim->Is_Finished()) { *bP = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bP]() { *bP = false; });
        };
    auto HitBox = [this](_int idx, _bool on) -> CBTNode* {
        return CBTAction::Create([this, idx, on](CBlackboard*, _float) {
            if (auto* p = static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()) p->Enable_HitBox(idx, on);
            return BT_STATUS::SUCCESS; });
        };
    auto PartnerAnim = [this](const _char* szClip, _bool bLoop) -> CBTNode* {
        return CBTAction::Create([this, szClip, bLoop](CBlackboard*, _float) {
            static_cast<CBoss_Armadillo*>(m_pOwner)->Play_PartnerAnim(szClip, bLoop);
            return BT_STATUS::SUCCESS; });
        };
    auto PartnerSpinHit = [this](_bool b) -> CBTNode* {
        return CBTAction::Create([this, b](CBlackboard*, _float) {
            static_cast<CBoss_Armadillo*>(m_pOwner)->Enable_PartnerSpinHit(b);
            return BT_STATUS::SUCCESS; });
        };

    // ---- 트윈 댄스: 파트너 들고 커비 향해 돌진, 벽에 맞으면 입사각 반사 (당구식) ----
    auto MakeTwinDance = [&]() -> CBTNode* {
        auto bOn = make_shared<bool>(false);
        auto fT = make_shared<_float>(0.f);
        auto fBounceCd = make_shared<_float>(0.f);
        auto iBounce = make_shared<_int>(0);
        auto vDir = make_shared<_float3>(_float3(0.f, 0.f, 1.f));
        const _float fDanceTime = 6.f;   // 총 지속 시간
        const _int   iMaxBounce = 4;     // 이만큼 반사했으면 종료
        const _float fSpinDeg = 540.f;

        auto* pRush = CBTAction::Create(
            [this, bOn, fT, fBounceCd, iBounce, vDir,
            fSpd, fRollSpeed, fWalkSpeed, fWallProbe, fDanceTime, iMaxBounce, fSpinDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
                auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                if (!*bOn) {
                    m_pOwner->Get_BodyAnimator()->Play("TwinDance", true, true, 0.1f, fSpd);
                    m_pOwner->Get_Movement()->Set_MoveSpeed(fRollSpeed);
                    m_pOwner->Get_Movement()->Set_LockFacing(true);   // 이동 방향 자동 바라보기 차단
                    pArma->Set_TwinDanceOffset(true);                 // 루트 = 두 마리 중점
                    *vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                    *fT = 0.f; *iBounce = 0; *fBounceCd = 0.f; *bOn = true;
                }
                *fT += dt;
                *fBounceCd -= dt;
                m_pOwner->Add_MoveDir(*vDir);

                m_pOwner->Get_Transform()->Rotate(
                    XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f),
                        XMConvertToRadians(fSpinDeg) * dt));

                _float3 vN{};
                if (*fBounceCd <= 0.f && pArma->Sweep_Wall(*vDir, fWallProbe, &vN)) {
                    _vector d = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(vDir.get()), 0.f));
                    _vector n = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vN), 0.f));
                    _vector r = d - n * (2.f * XMVectorGetX(XMVector3Dot(d, n)));
                    XMStoreFloat3(vDir.get(), XMVector3Normalize(XMVectorSetY(r, 0.f)));
                    ++(*iBounce);
                    *fBounceCd = 0.3f;
                }

                if (*fT >= fDanceTime || *iBounce >= iMaxBounce) {
                    m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
                    m_pOwner->Get_Movement()->Set_LockFacing(false);
                    pArma->Set_TwinDanceOffset(false);
                    *bOn = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [this, bOn, fWalkSpeed]() {
                *bOn = false;
                m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
                m_pOwner->Get_Movement()->Set_LockFacing(false);
                static_cast<CBoss_Armadillo*>(m_pOwner)->Set_TwinDanceOffset(false);
            });

        return CBTSequence::Create({
            PartnerAnim("TwinDanceStart", false),
            PlayFacing("TwinDanceStart", 360.f),
            PartnerAnim("TwinDance", true),
            HitBox(CBoss_Armadillo_Body::AHB_ROLL, true),
            PartnerSpinHit(true),
            pRush,
            PartnerSpinHit(false),
            HitBox(CBoss_Armadillo_Body::AHB_ROLL, false),
            PartnerAnim("TwinDanceEnd", false),
            OneShot("TwinDanceEnd"),
            });
        };

    // ---- 솔로 롤링: 벽 히트 시 정지 후 재조준 재돌진, 3번째 벽 = 그로기 ----
    auto MakeRoll = [&]() -> CBTNode* {
        auto bOn = make_shared<bool>(false);
        auto bGroggy = make_shared<bool>(false);
        auto iState = make_shared<_int>(0);      // 0 = 돌진, 1 = 재조준
        auto iWallHits = make_shared<_int>(0);
        auto fSegT = make_shared<_float>(0.f);
        auto fAimT = make_shared<_float>(0.f);
        auto vDir = make_shared<_float3>(_float3(0.f, 0.f, 1.f));
        const _int   iGroggyHits = 3;
        const _float fSegTimeMax = 4.f;    // 한 돌진이 벽을 못 만나면 이 시간 후 안전 종료
        const _float fReAimTime = 0.5f;
        const _float fSnapTurnDeg = 720.f;
        const _float fBounceBackSpeed = 8.f;
        const _float fBounceUpSpeed = 10.f;

        auto* pRush = CBTAction::Create(
            [this, bOn, bGroggy, iState, iWallHits, fSegT, fAimT, vDir,
            fSpd, fRollSpeed, fWalkSpeed, fWallProbe, iGroggyHits, fSegTimeMax, fReAimTime, fSnapTurnDeg
            , fBounceBackSpeed, fBounceUpSpeed](CBlackboard* pBB, _float dt) -> BT_STATUS {
                auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                if (!*bOn) {
                    m_pOwner->Get_BodyAnimator()->Play("RollAttack", true, true, 0.1f, fSpd);
                    m_pOwner->Get_Movement()->Set_MoveSpeed(fRollSpeed);
                    *vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                    *iState = 0; *iWallHits = 0; *fSegT = 0.f; *bGroggy = false; *bOn = true;
                }

                if (*iState == 0)   // 돌진 중
                {
                    *fSegT += dt;
                    m_pOwner->Add_MoveDir(*vDir);

                    _float3 vN{};
                    if (pArma->Sweep_Wall(*vDir, fWallProbe, &vN))
                    {
                        ++(*iWallHits);
                        if (*iWallHits >= iGroggyHits) {
                            _vector vBounce = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vN), 0.f));
                            m_pOwner->Get_Movement()->Launch(vBounce, fBounceBackSpeed, fBounceUpSpeed);

                            m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
                            *bGroggy = true; *bOn = false;
                            return BT_STATUS::SUCCESS;
                        }
                        *iState = 1; *fAimT = 0.f;                       // 이동 정지, 재조준으로
                        // TODO: 벽 충돌 셰이크 / 이펙트
                    }
                    else if (*fSegT >= fSegTimeMax) {                    // 허공 돌진 안전장치
                        m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
                        *bOn = false;
                        return BT_STATUS::SUCCESS;
                    }
                }
                else                // 재조준: 제자리에서 커비 쪽으로 휙 돌고 다시 돌진
                {
                    *fAimT += dt;

                    // 커비 방향으로 빠른 회전
                    _float3 vTo = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                    _vector vToTgt = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vTo), 0.f));
                    CTransform* pTf = m_pOwner->Get_Transform();
                    _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                    _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                    _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                        - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                    _float fYaw = atan2f(fCross, fDot);
                    _float fStep = XMConvertToRadians(fSnapTurnDeg) * dt;
                    _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                    pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));

                    if (*fAimT >= fReAimTime) {
                        XMStoreFloat3(vDir.get(), vToTgt);   // 재출발 방향 = 실제 타깃 방향
                        *iState = 0; *fSegT = 0.f;
                    }
                }
                return BT_STATUS::RUNNING;
            },
            [this, bOn, fWalkSpeed]() {
                *bOn = false;
                m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
            });

        auto bFallOn = make_shared<bool>(false);
        auto fFallT = make_shared<_float>(0.f);
        auto* pFallLoop = CBTAction::Create(
            [this, bFallOn, fFallT, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                if (!*bFallOn) {
                    m_pOwner->Get_BodyAnimator()->Play("HitWallFall", true, true, 0.1f, fSpd);
                    *fFallT = 0.f; *bFallOn = true;
                }
                *fFallT += dt;
                if (*fFallT >= 0.05f && !m_pOwner->Get_Movement()->Is_Launched()) {
                    *bFallOn = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [bFallOn]() { *bFallOn = false; });

            auto* pGroggy = CBTSequence::Create({
                CBTCondition::Create([bGroggy](CBlackboard*) { return *bGroggy; }),
                pFallLoop,
                OneShot("HitWallLanding"),
                HoldLoop("HitWallLoop", fGroggyTime),
                OneShot("HitWallEnd"),
            });

            return CBTSequence::Create({
                PlayFacing("RollChargeStart"),          
                OneShot("RollAttackStart"),
                HitBox(CBoss_Armadillo_Body::AHB_ROLL, true),
                pRush,                                 
                HitBox(CBoss_Armadillo_Body::AHB_ROLL, false),
                CBTSelector::Create({
                    pGroggy,                            
                    OneShot("RollBrake"),               
                    }),
            });
        };

    // ---- 파트너 던지기 1회: 소환 -> TwinRollingStart 모션 중 타이밍 맞춰 발사 ----
    auto MakePartnerThrow = [&]() -> CBTNode* {
        auto bOn = make_shared<bool>(false);

        auto* pThrow = CBTAction::Create(
            [this, bOn, fSpd, fTurnSpeedDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
                auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bOn) {
                    pAnim->Play("TwinRollingStart", false, true, 0.1f, fSpd);
                    pArma->Play_PartnerAnim("TwinRollingStart", false);
                    *bOn = true;
                }

                // 아직 안 던졌으면(파트너 보유 중) 커비 쪽으로 계속 회전
                if (pArma->Has_Partner()) {
                    _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                    _vector vToTgt = XMLoadFloat3(&vDir);
                    if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                        CTransform* pTf = m_pOwner->Get_Transform();
                        _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                        _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                        _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                            - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                        _float fYaw = atan2f(fCross, fDot);
                        _float fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
                        _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                        pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                    }
                }

                if (pAnim->Is_Finished()) { *bOn = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bOn]() { *bOn = false; });

        return CBTSequence::Create({
        CBTSelector::Create({
            CBTCondition::Create([this](CBlackboard*) {
                return static_cast<CBoss_Armadillo*>(m_pOwner)->Has_Partner(); }),
            PlayFacing("AppearPartnerShort2", 360.f),
            }),
        pThrow,
            });
        };

    // 잡기
    auto MakeCatch = [&]() -> CBTNode* {

        // 한 사이클
        auto MakeCatchOnce = [&](shared_ptr<bool> pThrown) -> CBTNode* {

            // (1) Walk 루프로 커비 추격, 사거리 안이면 SUCCESS (너무 오래 끌면 타임아웃)
            auto bWalk = make_shared<bool>(false);
            auto fWalkT = make_shared<_float>(0.f);
            const _float fSwingRange = 6.f;      // 이 거리 안이면 잡기 스윙
            const _float fChaseTimeout = 5.f;
            auto* pWalkChase = CBTAction::Create(
                [this, bWalk, fWalkT, fSpd, fWalkSpeed, fTurnSpeedDeg, fSwingRange, fChaseTimeout](CBlackboard* pBB, _float dt) -> BT_STATUS {
                    if (!*bWalk) {
                        m_pOwner->Get_BodyAnimator()->Play("Walk", true, true, 0.1f, fSpd);
                        m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
                        *fWalkT = 0.f; *bWalk = true;
                    }
                    *fWalkT += dt;

                    _float fDist = pBB->Get<_float>("DistToTarget", FLT_MAX);
                    if (fDist <= fSwingRange || *fWalkT >= fChaseTimeout) { *bWalk = false; return BT_STATUS::SUCCESS; }

                    _float3 vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 0.f));
                    _vector vToTgt = XMLoadFloat3(&vDir);
                    if (!XMVector3Equal(vToTgt, XMVectorZero())) {
                        m_pOwner->Add_MoveDir(vDir);
                        CTransform* pTf = m_pOwner->Get_Transform();
                        _vector vLook = XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f));
                        _float fDot = XMVectorGetX(XMVector3Dot(vLook, vToTgt));
                        _float fCross = XMVectorGetZ(vLook) * XMVectorGetX(vToTgt)
                            - XMVectorGetX(vLook) * XMVectorGetZ(vToTgt);
                        _float fYaw = atan2f(fCross, fDot);
                        _float fStep = XMConvertToRadians(fTurnSpeedDeg) * dt;
                        _float fApply = (fabsf(fYaw) <= fStep) ? fYaw : (fYaw > 0.f ? fStep : -fStep);
                        pTf->Rotate(XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), fApply));
                    }
                    return BT_STATUS::RUNNING;
                },
                [bWalk]() { *bWalk = false; });

            // (2) CatchAttack: 잡기 콜라이더 on, 히트나 종료까지 전진
            auto bOn = make_shared<bool>(false);
            auto* pCatchRush = CBTAction::Create(
                [this, bOn, fSpd](CBlackboard*, _float) -> BT_STATUS {
                    auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                    CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                    if (!*bOn) {
                        *bOn = true;
                        pArma->Reset_CatchHit();
                        pAnim->Play("CatchAttack", false, true, 0.1f, fSpd * 1.5f);
                        pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, true);
                    }

                    if (pArma->Is_CatchHit())
                        pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);

                    if (pAnim->Is_Finished()) {
                        pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);
                        *bOn = false;
                        return BT_STATUS::SUCCESS;
                    }
                    return BT_STATUS::RUNNING;
                },
                [this, bOn]() {
                    *bOn = false;
                    static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()
                        ->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);
                });

            // (3) QTE 대기: CatchSuccessWait 루프 5초. 탈출 성공->SUCCESS, 시간초과->FAILURE
            auto bWaitOn = make_shared<bool>(false);
            const _float fQTETime = 5.f;      // CatchSuccessWait 도달 후 탈출 제한시간
            auto* pQTEWait = CBTAction::Create(
                [this, bWaitOn, fQTETime, fSpd](CBlackboard*, _float dt) -> BT_STATUS {
                    auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                    if (!*bWaitOn) {
                        *bWaitOn = true;
                        pArma->Begin_QTE(fQTETime);   // 여기서 판정 시작(타이머 세팅 + escape 리셋 + QTE_Show)
                        m_pOwner->Get_BodyAnimator()->Play("CatchSuccessWait", true, true, 0.1f, fSpd);
                    }
                    if (pArma->Is_QTEEscaped()) { *bWaitOn = false; pArma->End_QTE(); return BT_STATUS::SUCCESS; }
                    if (pArma->Tick_QTETimer(dt)) { *bWaitOn = false; pArma->End_QTE(); return BT_STATUS::FAILURE; }
                    return BT_STATUS::RUNNING;
                },
                [this, bWaitOn]() { *bWaitOn = false; static_cast<CBoss_Armadillo*>(m_pOwner)->End_QTE(); });

            auto* pReleaseEscape = CBTAction::Create([this](CBlackboard*, _float) {
                static_cast<CBoss_Armadillo*>(m_pOwner)->Fire_Release(KIRBY_ATTACHMENT_END_REASON::GORILLA_COMBAT_ESCAPE);
                return BT_STATUS::SUCCESS; });

            // CatchThrow: 던지기 재생 후 커비 SLAM 상태로 릴리즈
            auto bThrowOn = make_shared<bool>(false);
            auto* pThrow = CBTAction::Create(
                [this, bThrowOn, fSpd](CBlackboard*, _float) -> BT_STATUS {
                    CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                    if (!*bThrowOn) {
                        *bThrowOn = true;
                        pAnim->Play("CatchThrow", false, true, 0.1f, fSpd);
                    }
                    if (pAnim->Is_Finished()) {
                        *bThrowOn = false;
                        static_cast<CBoss_Armadillo*>(m_pOwner)->Fire_Release(KIRBY_ATTACHMENT_END_REASON::GORILLA_COMBAT_THROWN);
                        return BT_STATUS::SUCCESS;
                    }
                    return BT_STATUS::RUNNING;
                },
                [bThrowOn]() { *bThrowOn = false; });

            return CBTSequence::Create({
                // 결과 플래그 리셋(루프 재진입 대비)
                CBTAction::Create([pThrown](CBlackboard*, _float) {
                    *pThrown = false; return BT_STATUS::SUCCESS; }),
                pWalkChase,
                PlayFacing("CatchAttackStart", 120.f, 2.f),
                pCatchRush,
                CBTSelector::Create({
                    // 잡힘 -> CatchSuccess -> QTE
                    CBTSequence::Create({
                        CBTCondition::Create([this](CBlackboard*) {
                            return static_cast<CBoss_Armadillo*>(m_pOwner)->Is_CatchHit(); }),
                        OneShot("CatchSuccess"),
                        CBTSelector::Create({
                            // 탈출 성공(pQTEWait SUCCESS) -> CatchRelease (계속)
                            CBTSequence::Create({ pQTEWait, pReleaseEscape, OneShot("CatchRelease") }),
                            // 시간초과(pQTEWait FAILURE) -> CatchThrow -> 던짐 표시(종료)
                            CBTSequence::Create({
                                pThrow,
                                CBTAction::Create([pThrown](CBlackboard*, _float) {
                                    *pThrown = true; return BT_STATUS::SUCCESS; }),
                                }),
                            }),
                        }),
                        // 미스 -> CatchFailure (계속)
                        OneShot("CatchFailure"),
                        }),
                });
            };

            auto bThrown1 = make_shared<bool>(false);
            auto bThrown2 = make_shared<bool>(false);

            return CBTSequence::Create({
                CBTAction::Create([this](CBlackboard*, _float) {
                    static_cast<CBoss_Armadillo*>(m_pOwner)->Show_Cage();
                    return BT_STATUS::SUCCESS; }),
                OneShot("CarryStart"),

                    // 1번째 잡기
                    MakeCatchOnce(bThrown1),
                    CBTSelector::Create({
                    // 잡힘+QTE실패(던짐) -> 즉시 종료 (CarryEnd 없음, 케이지는 CatchThrow 애님이벤트로 숨김)
                    CBTCondition::Create([bThrown1](CBlackboard*) { return *bThrown1; }),

                    // 탈출 성공 or 미스 -> 2번째 잡기
                    CBTSequence::Create({
                        MakeCatchOnce(bThrown2),
                        CBTSelector::Create({
                            // 2번째도 던짐 -> 즉시 종료 (CarryEnd 없음)
                            CBTCondition::Create([bThrown2](CBlackboard*) { return *bThrown2; }),
                            // 2번째 탈출/미스 -> 정상 종료 (CarryEnd + 케이지 숨김)
                            CBTSequence::Create({
                                OneShot("CarryEnd"),
                                CBTAction::Create([this](CBlackboard*, _float) {
                                    static_cast<CBoss_Armadillo*>(m_pOwner)->Hide_Cage();
                                    return BT_STATUS::SUCCESS; }),
                                }),
                            }),
                        }),
                    }),
                });
        };

    // ---- 조립 ----

    // 오프닝(1회): 파트너 소환 -> 트윈롤링 -> 파트너 던지기 -> 솔로 롤링
    auto* pOpening = CBTSequence::Create({
        PlayFacing("AppearPartnerShort2", 360.f),
        MakeTwinDance(),
        MakePartnerThrow(),
        MakePartnerThrow(),
        MakePartnerThrow(),
        CBTAction::Create([this](CBlackboard*, _float) {
        m_bOpeningDone = true; return BT_STATUS::SUCCESS; }),
        MakeRoll(),
        });

    // 루프: (근접이면 잡기 / 아니면 파트너 소환+던지기) -> 솔로 롤링
    auto* pLoop = CBTSequence::Create({
        Rest(),
        CBTSelector::Create({
            CBTSequence::Create({
                /*CBTCondition::Create([](CBlackboard* pBB) {
                    return pBB->Get<_float>("DistToTarget", FLT_MAX) <= 25.f; }),*/
                CBTCondition::Create([](CBlackboard* pBB) {
                    return true; }),          // [임시]
                MakeCatch(),
                }),
            CBTSequence::Create({
                MakePartnerThrow(),
                MakePartnerThrow(),
                MakePartnerThrow(),
                }),
            }),
        Rest(),
        MakeRoll(),
        });

    return CBTSelector::Create({
        CBTSequence::Create({
            CBTCondition::Create([this](CBlackboard*) { return !m_bOpeningDone; }),
            pOpening,
            }),
        pLoop,
        });
}

CBoss_Armadillo_Brain* CBoss_Armadillo_Brain::Create(CMonster* pOwner)
{
    CBoss_Armadillo_Brain* pInstance = new CBoss_Armadillo_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Armadillo_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Armadillo_Brain::Free()
{
    __super::Free();
}