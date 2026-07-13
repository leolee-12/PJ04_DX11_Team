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

    // ---- 튜닝 상수 ----
    const _float fSpd = 1.f;
    const _float fCatchRange = 8.f;      // 루프에서 잡기를 고르는 거리
    const _float fTurnSpeedDeg = 120.f;
    const _float fChargeTime = 0.6f;     // 차지 루프 유지 시간
    const _float fWalkSpeed = 4.f;      // 기본 이동 속도 (Initialize와 일치)
    const _float fRollSpeed = 14.f;     // 구르기 돌진 속도
    const _float fGroggyTime = 3.f;      // 3벽 그로기 시간
    const _float fWallProbe = 3.5f;     // 전방 벽 감지 스윕 거리
    const _float fRestTime = 0.8f;     // 패턴 사이 숨 고르기

    // ---- 공용 헬퍼 ----
    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };
    auto OneShot = [&](const string& c) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, false, 0.f, fSpd });
        };
    auto HoldLoop = [&](const string& c, _float fHold) -> CBTNode* {
        return CBTPlayClip::Create(Anim, { c, true, fHold, fSpd });
        };
    auto Rest = [&]() -> CBTNode* { return HoldLoop("Wait", fRestTime); };
    auto PlayFacing = [&](const string& c, _float fTurnDeg = 120.f) -> CBTNode* {
        auto bP = make_shared<bool>(false);
        return CBTAction::Create(
            [this, bP, c, fSpd, fTurnDeg](CBlackboard* pBB, _float dt) -> BT_STATUS {
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bP) { pAnim->Play(c, false, true, 0.1f, fSpd); *bP = true; }

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
    auto BossCall = [this](void (CBoss_Armadillo::* fn)()) -> CBTNode* {
        return CBTAction::Create([this, fn](CBlackboard*, _float) {
            (static_cast<CBoss_Armadillo*>(m_pOwner)->*fn)();
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

        auto* pRush = CBTAction::Create(
            [this, bOn, fT, fBounceCd, iBounce, vDir,
            fSpd, fRollSpeed, fWalkSpeed, fWallProbe, fDanceTime, iMaxBounce](CBlackboard* pBB, _float dt) -> BT_STATUS {
                auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                if (!*bOn) {
                    m_pOwner->Get_BodyAnimator()->Play("TwinDance", true, true, 0.1f, fSpd);
                    m_pOwner->Get_Movement()->Set_MoveSpeed(fRollSpeed);
                    *vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                    *fT = 0.f; *iBounce = 0; *fBounceCd = 0.f; *bOn = true;
                }
                *fT += dt;
                *fBounceCd -= dt;
                m_pOwner->Add_MoveDir(*vDir);

                _float3 vN{};
                if (*fBounceCd <= 0.f && pArma->Sweep_Wall(*vDir, fWallProbe, &vN)) {
                    _vector d = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(vDir.get()), 0.f));
                    _vector n = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vN), 0.f));
                    _vector r = d - n * (2.f * XMVectorGetX(XMVector3Dot(d, n)));
                    XMStoreFloat3(vDir.get(), XMVector3Normalize(XMVectorSetY(r, 0.f)));
                    ++(*iBounce);
                    *fBounceCd = 0.3f;   // 같은 벽 중복 감지 방지
                    // TODO: 반사 순간 카메라 셰이크 / 이펙트 / SFX
                }

                if (*fT >= fDanceTime || *iBounce >= iMaxBounce) {
                    m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
                    *bOn = false;
                    return BT_STATUS::SUCCESS;
                }
                return BT_STATUS::RUNNING;
            },
            [this, bOn, fWalkSpeed]() {
                *bOn = false;
                m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
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
        const _float fReAimTime = 0.45f;  // 벽 히트 후 제자리 재조준 시간

        auto* pRush = CBTAction::Create(
            [this, bOn, bGroggy, iState, iWallHits, fSegT, fAimT, vDir,
            fSpd, fRollSpeed, fWalkSpeed, fWallProbe, iGroggyHits, fSegTimeMax, fReAimTime](CBlackboard* pBB, _float dt) -> BT_STATUS {
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
                        if (*iWallHits >= iGroggyHits) {                 // 3번째 벽: 그로기
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
                else                // 재조준: 제자리에서 잠깐 돈 뒤 다시 커비 향해 발사
                {
                    *fAimT += dt;
                    if (*fAimT >= fReAimTime) {
                        *vDir = pBB->Get<_float3>("DirToTarget", _float3(0.f, 0.f, 1.f));
                        *iState = 0; *fSegT = 0.f;
                    }
                }
                return BT_STATUS::RUNNING;
            },
            [this, bOn, fWalkSpeed]() {
                *bOn = false;
                m_pOwner->Get_Movement()->Set_MoveSpeed(fWalkSpeed);
            });

        // 3벽 그로기: 클립 순서는 추정이므로 에디터에서 확인 후 조정
        auto* pGroggy = CBTSequence::Create({
            CBTCondition::Create([bGroggy](CBlackboard*) { return *bGroggy; }),
            OneShot("HitWallBlowStart"),
            OneShot("HitWallFall"),
            OneShot("HitWallLanding"),
            HoldLoop("HitWallLoop", fGroggyTime),   // 뒤집혀 버둥 = 무방비 개방
            OneShot("HitWallEnd"),
            });

        return CBTSequence::Create({
            PlayFacing("RollChargeStart"),          // 제자리 돌진 준비 + 조준
            HoldLoop("RollChargeLoop", fChargeTime),
            OneShot("RollAttackStart"),
            HitBox(CBoss_Armadillo_Body::AHB_ROLL, true),
            pRush,
            HitBox(CBoss_Armadillo_Body::AHB_ROLL, false),
            CBTSelector::Create({
                pGroggy,                            // 3번 박았으면 그로기
                OneShot("RollBrake"),               // 아니면 브레이크
                }),
            });
        };

    // ---- 파트너 던지기 1회: 소환 -> TwinRollingStart 모션 중 타이밍 맞춰 발사 ----
    auto MakePartnerThrow = [&]() -> CBTNode* {
        auto bOn = make_shared<bool>(false);
        auto bFired = make_shared<bool>(false);
        auto fT = make_shared<_float>(0.f);
        const _float fFireDelay = 0.35f;   // 클립 시작 후 발사까지 (릴리스 프레임에 맞춰 튜닝)
        const _float fAimTurnDeg = 360.f;

        auto* pThrow = CBTAction::Create(
            [this, bOn, bFired, fT, fSpd, fTurnSpeedDeg, fFireDelay](CBlackboard* pBB, _float dt) -> BT_STATUS {
                auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bOn) {
                    pAnim->Play("TwinRollingStart", false, true, 0.1f, fSpd);
                    pArma->Play_PartnerAnim("TwinRollingStart", false);
                    *fT = 0.f; *bFired = false; *bOn = true;
                }
                *fT += dt;

                // 발사 전까지는 커비 쪽으로 조준 회전
                if (!*bFired) {
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
                    if (*fT >= fFireDelay) { pArma->Fire_PartnerThrow(); *bFired = true; }
                }

                if (pAnim->Is_Finished()) { *bOn = false; return BT_STATUS::SUCCESS; }
                return BT_STATUS::RUNNING;
            },
            [bOn, bFired, fT]() { *bOn = false; *bFired = false; *fT = 0.f; });

        return CBTSequence::Create({
            pThrow,
            });
        };

    // ---- 잡기 (간소판: 명중 시 껴안았다 내던지기) ----
    auto MakeCatch = [&]() -> CBTNode* {
        auto bOn = make_shared<bool>(false);
        auto* pCatchRush = CBTAction::Create(
            [this, bOn, fSpd](CBlackboard*, _float) -> BT_STATUS {
                auto* pArma = static_cast<CBoss_Armadillo*>(m_pOwner);
                CAnimator* pAnim = m_pOwner->Get_BodyAnimator();
                if (!*bOn) {
                    *bOn = true;
                    pArma->Reset_CatchHit();
                    pAnim->Play("CatchAttack", false, true, 0.1f, fSpd);
                    pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, true);
                }
                if (pArma->Is_CatchHit() || pAnim->Is_Finished()) {
                    pArma->Get_HitBoxPart()->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);
                    *bOn = false;
                    return BT_STATUS::SUCCESS;
                }
                // 전진하며 덮치기
                CTransform* pTf = m_pOwner->Get_Transform();
                _float3 vFwd;
                XMStoreFloat3(&vFwd, XMVector3Normalize(XMVectorSetY(pTf->Get_State(STATE::LOOK), 0.f)));
                m_pOwner->Add_MoveDir(vFwd);
                return BT_STATUS::RUNNING;
            },
            [this, bOn]() {
                *bOn = false;
                static_cast<CBoss*>(m_pOwner)->Get_HitBoxPart()
                    ->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);
            });

        return CBTSequence::Create({
            PlayFacing("CatchAttackStart"),
            pCatchRush,
            CBTSelector::Create({
                CBTSequence::Create({
                    CBTCondition::Create([this](CBlackboard*) {
                        return static_cast<CBoss_Armadillo*>(m_pOwner)->Is_CatchHit(); }),
                    OneShot("CatchSuccess"),
                    HoldLoop("CatchSuccessWait", 0.8f),
                    HitBox(CBoss_Armadillo_Body::AHB_THROW, true),
                    OneShot("CatchThrow"),
                    HitBox(CBoss_Armadillo_Body::AHB_THROW, false),
                    }),
                OneShot("CatchFailure"),            // 빗나감: 후딜 개방
                }),
            });
        };

    // ---- 조립 ----

    // 오프닝(1회): 파트너 소환 -> 트윈롤링 -> 파트너 던지기 -> 솔로 롤링
    auto* pOpening = CBTSequence::Create({
        BossCall(&CBoss_Armadillo::Summon_Partner),
        PlayFacing("AppearPartnerShort2", 360.f),  
        MakeTwinDance(),
        MakePartnerThrow(),                           
        MakePartnerThrow(),                           
        MakePartnerThrow(),                           
        CBTAction::Create([this](CBlackboard*, _float) {
            m_bOpeningDone = true; return BT_STATUS::SUCCESS; }),
        });

    // 루프: (근접이면 잡기 / 아니면 파트너 소환+던지기) -> 솔로 롤링
    auto* pLoop = CBTSequence::Create({
        Rest(),
        CBTSelector::Create({
            CBTSequence::Create({
                CBTCondition::Create([fCatchRange](CBlackboard* pBB) {
                    return pBB->Get<_float>("DistToTarget", FLT_MAX) <= fCatchRange; }),
                MakeCatch(),
                }),
            CBTSequence::Create({
                PlayFacing("AppearPartnerShort2", 360.f),
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