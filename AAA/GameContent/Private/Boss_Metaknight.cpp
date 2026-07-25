#include "Boss_Metaknight.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Metaknight_Brain.h"
#include "Boss_Metaknight_Body.h"
#include "Boss_Metaknight_ReplicaSword.h"
#include "Boss_Metaknight_Sword.h"
#include "Boss_Metaknight_Mant.h"
#include "Boss_Metaknight_EscapeMant.h"

#include "AttackDecal.h"

#include "Projectile_Manager.h"
#include "Projectile_MoonShot.h"
#include "Projectile_Rock.h"

#include "Effect_Loader.h"

const _float3 CBoss_Metaknight::s_vGigaPoints[CBoss_Metaknight::GIGA_POINT_COUNT] = {
    { 20.5f, 7.23f, 15.f },
    { 20.5f, 7.23f, -15.f },
    { -20.5f, 7.23f, 15.f },
    { -20.5f, 7.23f, -15.f },
};

const vector<_float> CBoss_Metaknight::s_Thresholds = { 0.5f };

namespace
{
    _matrix Strip_Scale(_fmatrix m)
    {
        _matrix r = m;
        r.r[0] = XMVector3Normalize(m.r[0]);
        r.r[1] = XMVector3Normalize(m.r[1]);
        r.r[2] = XMVector3Normalize(m.r[2]);
        return r;
    }

    void Sort_CornersPerimeter(_float3 corners[4])
    {
        _float cx = 0.f, cz = 0.f;
        for (int i = 0; i < 4; ++i) { cx += corners[i].x; cz += corners[i].z; }
        cx *= 0.25f; cz *= 0.25f;

        std::sort(corners, corners + 4, [cx, cz](const _float3& a, const _float3& b) {
            return atan2f(a.z - cz, a.x - cx) < atan2f(b.z - cz, b.x - cx);
            });
    }

    _bool Is_SwordHit(HIT_TYPE eHitType)
    {
        return eHitType == HIT_TYPE::SWORD_DEFAULT
            || eHitType == HIT_TYPE::SWORD_SPIN
            || eHitType == HIT_TYPE::UPWARD_SLASH;
    }
}

CBoss_Metaknight::CBoss_Metaknight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBoss(pDevice, pContext) {
}
CBoss_Metaknight::CBoss_Metaknight(const CBoss_Metaknight& Prototype)
    : CBoss(Prototype) {
}

HRESULT CBoss_Metaknight::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Metaknight::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_MetaEvents()))
        return E_FAIL;

    m_strBossName = L"메타나이트";
    m_fMaxHP = 100.f;
    m_fCurHP = m_fMaxHP;

    m_pTransformCom->Set_Scale(1.3f, 1.3f, 1.3f);

    m_pMovement->Set_MoveSpeed(6.f);
    m_pMovement->Set_RotSpeed(240.f);
    m_pMovement->Set_Acceleration(10000.f, 10000.f);

    m_pBody->Get_Animator()->Play("Wait", true, false, 0.f, 1.f);

    return S_OK;
}

void CBoss_Metaknight::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        if (m_pMovement) m_pMovement->Sync_To_Controller();
        return;
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_0))
        Appear();
    if (m_pGameInstance_Proxy->Key_Down(DIK_9))
        Die();
#endif
    Tick_Cooldowns(fTimeDelta);
    Tick_Locking(fTimeDelta);
    Tick_Appear(fTimeDelta);
    Tick_DeathSequence(fTimeDelta);

    __super::Update(fTimeDelta);

    if (Is_PhaseTransitioning())
        Update_PhaseTransition(fTimeDelta);
}

void CBoss_Metaknight::Late_Update(_float fTimeDelta)
{
    Update_Attachment();
    __super::Late_Update(fTimeDelta);
}

CMonsterBrain* CBoss_Metaknight::Create_Brain()
{
    return CBoss_Metaknight_Brain::Create(this);
}

void CBoss_Metaknight::Play_Intro()
{
    if (m_pBody) m_pBody->Set_Active(true);
    if (m_pMant) m_pMant->Set_Active(true);
    Set_ActiveSword(m_eActiveSword);

    m_pController->Set_Solid(false);

    if (s_bSkipIntro)
    {
        if (CAnimator* pAnim = Get_BodyAnimator())
            pAnim->Play("Wait", true, true, 0.f, s_fDefaultAnimSpeed);
        return;
    }

    KIRBY_POSITION_SYNC_BEGIN_DESC Sync{};
    Sync.eType = KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_INTRO;
    XMStoreFloat4x4(&Sync.AnchorWorld,
        Strip_Scale(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
    Sync.fAnimSpeed = s_fDefaultAnimSpeed;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncBegin, &Sync);

    static constexpr const _char* INTRO_CUTS[] = {
        "DemoAppearCut1", "DemoAppearCut2", "DemoAppearCut3", "DemoAppearCut4", "DemoAppearCut5",
    };

    CAnimator* pBodyAnim = Get_BodyAnimator();
    CAnimator* pMantAnim = (m_pMant && m_pMant->Is_Active()) ? m_pMant->Get_Animator() : nullptr;
    if (!pBodyAnim) return;

    pBodyAnim->Play(INTRO_CUTS[0], false, true, 0.f, s_fDefaultAnimSpeed);
    if (pMantAnim) pMantAnim->Play(INTRO_CUTS[0], false, true, 0.f, s_fDefaultAnimSpeed);

    for (_uint i = 1; i < _countof(INTRO_CUTS); ++i)
    {
        CAnimator::ANI_PLAY_INFO tInfo{};
        tInfo.strAniName = INTRO_CUTS[i];
        tInfo.bLoop = false;
        tInfo.bRestart = true;
        tInfo.fBlend = 0.f;
        tInfo.fSpeed = s_fDefaultAnimSpeed;

        pBodyAnim->Enqueue(tInfo);
        if (pMantAnim) pMantAnim->Enqueue(tInfo);
    }
}

_bool CBoss_Metaknight::Is_Intro_Finished() const
{
    if (s_bSkipIntro)
        return true;

    return m_fIntroHoldTimer >= INTRO_HOLD_TIME;
}

void CBoss_Metaknight::On_Intro_End()
{
    if (m_pBody)
    {
        const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("TopL");
        if (pBone)
        {
            _matrix matBoneWorld = XMLoadFloat4x4(pBone)
                * XMLoadFloat4x4(m_pBody->Get_CombinedWorldMatrixPtr());
            _vector vBonePos = matBoneWorld.r[3];

            _vector vCur = m_pTransformCom->Get_State(STATE::POSITION);
            _vector vNew = XMVectorSetY(vBonePos, XMVectorGetY(vCur));

            m_pTransformCom->Set_State(STATE::POSITION, vNew);
            if (m_pController)
                m_pController->Set_FootPosition(vNew);
        }
    }

    //METAKNIGHT_INTRO_END;

    KIRBY_POSITION_SYNC_END_DESC  SyncEnd{ KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_INTRO_END };
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncEnd, &SyncEnd);

    Show_Mant(false);

    if (m_pController)
        m_pController->Set_Solid(false);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);

    BOSSCAM_CONFIG_DESC cfg{};
    cfg.fAimHeight = 0.f;
    cfg.fHeight = 4.f;
    cfg.fShoulderOffset = 0.f;
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_Config, &cfg);
}

void CBoss_Metaknight::Play_Death()
{
    Enable_Colliders(false);
    if (auto* p = Get_HitBoxPart())
        p->Enable_AllHitBoxes(false);

    Detach_FromKirby();
    Set_ParryWindow(false);
    Enable_CatchBox(false);
    End_LockingSync();

    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return;

    pAnim->Play("Damage", false, true, 0.f, s_fDefaultAnimSpeed);

    _vector vDir = XMVectorSetY(
        XMLoadFloat3(&Get_BlackBoard().vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        m_pTransformCom->LookTo(XMVector3Normalize(vDir));

    m_pGameInstance_Proxy->Publish(EventTag::FullScreen_Flash, nullptr);

    m_eDeathStep = EDEATH::POSE_WAIT;
    m_iDeathPoseDelay = 2;
}

_bool CBoss_Metaknight::Is_Death_Finished() const
{
    return m_eDeathStep == EDEATH::DONE;
}

void CBoss_Metaknight::On_Enter_Corpse()
{
    __super::On_Enter_Corpse();

    m_pGameInstance_Proxy->Publish(EventTag::Level_BossDefeated, nullptr);
}

void CBoss_Metaknight::Play_PhaseTransition(_int iNewPhase)
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Damage2", false, true, 0.1f, s_fDefaultAnimSpeed);

    m_fPhaseBaseY = XMVectorGetY(m_pTransformCom->Get_State(STATE::POSITION));
    m_fPhaseVelY = sqrtf(2.f * PHASE_HOP_GRAVITY * PHASE_HOP_HEIGHT);
    m_ePhaseTrans = EPhaseTrans::HOP;
    m_pMovement->Set_GravityEnabled(false);
}

_bool CBoss_Metaknight::Is_PhaseTransition_Finished() const
{
    return m_ePhaseTrans == EPhaseTrans::DONE;
}

void CBoss_Metaknight::On_PhaseChanged(_int iOldPhase, _int iNewPhase)
{
    if (iNewPhase == 1)
    {
        m_fRockCooldown = 0.f;
    }
}

_bool CBoss_Metaknight::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f;
    Out.fHeight = s_fCCT_Height + 0.1f;
    return true;
}

CAnimator* CBoss_Metaknight::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMultiHitBoxPart* CBoss_Metaknight::Get_HitBoxPart() const
{
    return m_pBody;
}

HRESULT CBoss_Metaknight::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        if (Handle_SoundAnimEvent(e, phase))
            return;
        if (Handle_FxAnimEvent(e, phase))
            return;

        switch (static_cast<EANIM_EVENT>(e.iEventType))
        {
            case EANIM_EVENT::CamTrack:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                if (!e.strParam.empty())
                {
                    wstring w = StrToWstr(e.strParam);
                    Fire_CutsceneCamera(w.c_str());
                }
                else
                {
                    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
                    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
                }
                break;
            }

            case EANIM_EVENT::PubEvent:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;
                if (e.strParam.empty()) break;

                wstring w = StrToWstr(e.strParam);

                void* pPayload = (e.iIntParam == 1)
                    ? const_cast<_float4x4*>(m_pTransformCom->Get_WorldMatrixPtr())
                    : nullptr;

                m_pGameInstance_Proxy->Publish(w.c_str(), pPayload);
                break;
            }

            case EANIM_EVENT::OnOffPart:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                switch (e.iIntParam)
                {
                    case 0:
                        Hide_Sword(m_eActiveSword);
                        break;
                    case 1:
                        Show_Mant(false);
                        break;
                    default:
                        break;
                }
                break;
            }

            case EANIM_EVENT::OnOffMesh:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;
                
                m_pBody->OffMask();
                break;
            }
        }

        });
    return S_OK;
}

HRESULT CBoss_Metaknight::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Metaknight_Body>(
        CBoss_Metaknight_Body::PROTOTYPE_TAG, CBoss_Metaknight_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;

    m_pSword = Add_MonsterPart<CBoss_Metaknight_Sword>(
        CBoss_Metaknight_Sword::PROTOTYPE_TAG, CBoss_Metaknight_Sword::PART_TAG,
        m_pBody->Get_BoneMatrixPtr("RHaveL"));
    if (!m_pSword) return E_FAIL;
    m_pSword->Set_IgnoreSocketScale(true);

    m_pReplica = Add_MonsterPart<CBoss_Metaknight_ReplicaSword>(
        CBoss_Metaknight_ReplicaSword::PROTOTYPE_TAG, CBoss_Metaknight_ReplicaSword::PART_TAG,
        m_pBody->Get_BoneMatrixPtr("RHaveL"));
    if (!m_pReplica) return E_FAIL;
    m_pReplica->Set_IgnoreSocketScale(true);

    m_pMant = Add_MonsterPart<CBoss_Metaknight_Mant>(
        CBoss_Metaknight_Mant::PROTOTYPE_TAG, CBoss_Metaknight_Mant::PART_TAG);
    if (!m_pMant) return E_FAIL;

    m_pEscapeMant = Add_MonsterPart<CBoss_Metaknight_EscapeMant>(
        CBoss_Metaknight_EscapeMant::PROTOTYPE_TAG, CBoss_Metaknight_EscapeMant::PART_TAG);
    if (!m_pEscapeMant) return E_FAIL;

    constexpr _float fMantBakedScale = 1.f / 1.3f;
    m_pMant->Get_Transform()->Set_Scale(fMantBakedScale, fMantBakedScale, fMantBakedScale);
    m_pEscapeMant->Get_Transform()->Set_Scale(fMantBakedScale, fMantBakedScale, fMantBakedScale);
    m_pEscapeMant->Set_Active(false);

    Set_ActiveSword(EMK_SWORD::GALAXIA);

    m_pBody->Set_HitBox_OnEnter(CBoss_Metaknight_Body::MKHB_CATCH,
        [this](CCollider* pOther)
        {
            if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
                return;
            if (m_bCatchHit)
                return;

            m_bCatchHit = true;
            m_pBody->Enable_HitBox(CBoss_Metaknight_Body::MKHB_CATCH, false);
        });

    return S_OK;
}

const _float4x4* CBoss_Metaknight::Get_FxParentMatrix(const _wstring& strFx) const
{
    return m_pTransformCom->Get_WorldMatrixPtr();
}

void CBoss_Metaknight::Damaged(const ATTACK_INFO& tInfo)
{
    if (m_Lock.bParryWindow && Is_SwordHit(tInfo.eHitType))
    {
        m_pGameInstance_Proxy->Publish(EventTag::Metaknight_ParryBegin, nullptr);

        Enter_Locking();
        return;
    }

    if (m_bDodgeInvuln)
        return;

    if (!m_bAttackBusy && m_fDodgeCooldown <= 0.f)
    {
        m_bDodgeRequested = true;
        return;
    }

    __super::Damaged(tInfo);
}

void CBoss_Metaknight::Update_AI(_float fTimeDelta)
{
    if (m_Lock.bActive || m_Lock.bAttached)
    {
        Clear_MoveDir();
        return;
    }

    __super::Update_AI(fTimeDelta);
}

void CBoss_Metaknight::Set_ActiveSword(EMK_SWORD eSword)
{
    m_eActiveSword = eSword;

    if (m_pSword)
        m_pSword->Set_Drawn(eSword == EMK_SWORD::GALAXIA);
    if (m_pReplica)
        m_pReplica->Set_Drawn(eSword == EMK_SWORD::REPLICA);
}

void CBoss_Metaknight::Enable_SwordHit(_bool bOn)
{
    switch (m_eActiveSword)
    {
        case EMK_SWORD::GALAXIA: if (m_pSword)   m_pSword->Set_HitBox(bOn);   break;
        case EMK_SWORD::REPLICA: if (m_pReplica) m_pReplica->Set_HitBox(bOn); break;
        default: break;
    }
}

void CBoss_Metaknight::Hide_Sword(EMK_SWORD eSword)
{
    switch (eSword)
    {
        case EMK_SWORD::GALAXIA:
            if (m_pSword)
                m_pSword->Set_Drawn(false);
            break;
        case EMK_SWORD::REPLICA:
            if (m_pReplica)
                m_pReplica->Set_Drawn(false);
            break;
        default:
            break;
    }
}

void CBoss_Metaknight::Drop_Galaxia()
{
    if (m_pSword)
        m_pSword->Drop_Freeze();
}

void CBoss_Metaknight::Retire_Galaxia()
{
    if (m_pSword)
        m_pSword->Retire_Drop();
}

void CBoss_Metaknight::Hold_BossCam(_bool bOn)
{
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_Hold, &bOn);
}

void CBoss_Metaknight::Show_Mant(_bool bOn)
{
    if (m_pMant)
        m_pMant->Set_Active(bOn);
}

void CBoss_Metaknight::Play_MantSync(const _char* szClip, _bool bLoop, _float fBland, _float fSpeed)
{
    if (m_pMant && m_pMant->Is_Active())
        m_pMant->Get_Animator()->Play(szClip, bLoop, true, fBland, fSpeed);
}

void CBoss_Metaknight::Show_EscapeMant(_bool bOn)
{
    if (m_pEscapeMant)
        m_pEscapeMant->Set_Active(bOn);
}

void CBoss_Metaknight::Play_EscapeMantSequence()
{
    if (nullptr == m_pEscapeMant || !m_pEscapeMant->Is_Active())
        return;

    CAnimator* pAnim = m_pEscapeMant->Get_Animator();
    if (nullptr == pAnim)
        return;

    pAnim->Play("EscapeStart", false, true, 0.f, s_fDefaultAnimSpeed);

    CAnimator::ANI_PLAY_INFO tInfo{};
    tInfo.strAniName = "Escape";
    tInfo.bLoop = false;
    tInfo.bRestart = true;
    tInfo.fBlend = 0.f;
    tInfo.fSpeed = s_fDefaultAnimSpeed;
    pAnim->Enqueue(tInfo);
}

void CBoss_Metaknight::Fire_GigaMoonShot()
{
    CProjectile* p = nullptr;
    CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(),
        CProjectile_MoonShot::POOL_KEY, CProjectile_MoonShot::PROTOTYPE_TAG, &p);
    if (!p) return;

    _vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vDir = XMVectorSetY(XMLoadFloat3(&Get_BlackBoard().vTargetPos) - vSelf, 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-6f)
        vDir = m_pTransformCom->Get_State(STATE::LOOK);
    vDir = XMVector3Normalize(vDir);

    _float3 vPos; XMStoreFloat3(&vPos, vSelf + vDir * 1.5f + XMVectorSet(0.f, 1.2f, 0.f, 0.f));
    _float3 vD;   XMStoreFloat3(&vD, vDir);
    p->Launch(vPos, vD);
}

void CBoss_Metaknight::Begin_RockDecalSlide()
{
    m_Rock.Build(s_vGigaPoints);
    m_Rock.Select_Safe(m_pGameInstance_Proxy);

    _vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
    const _float sx = XMVectorGetX(vSelf), sz = XMVectorGetZ(vSelf);

    for (int i = 0; i < ROCK_FIELD::TILE_COUNT; ++i)
    {
        if (m_Rock.bSafe[i])
        {
            if (m_Rock.pDecals[i]) m_Rock.pDecals[i]->Set_Active(false);
            continue;
        }

        if (nullptr == m_Rock.pDecals[i])
        {
            CGameObject* pObj = nullptr;
            if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj,
                m_iPrototypeLevel, CAttackDecal::PROTOTYPE_TAG,
                m_iPrototypeLevel, TEXT("Layer_Effect"), TEXT("RockDecal"), nullptr)))
                continue;

            m_Rock.pDecals[i] = dynamic_cast<CAttackDecal*>(pObj);
            if (nullptr == m_Rock.pDecals[i]) continue;
        }
        else m_Rock.pDecals[i]->Set_Active(true);

        _float3 vStart = { sx, m_Rock.vTiles[i].y, sz };
        m_Rock.pDecals[i]->Place(vStart, ROCK_DECAL_RADIUS, 9999.f);
        m_Rock.pDecals[i]->Slide_To(m_Rock.vTiles[i], ROCK_SLIDE_TIME);
    }
}

void CBoss_Metaknight::Drop_Rocks()
{
    for (int i = 0; i < ROCK_FIELD::TILE_COUNT; ++i)
    {
        if (m_Rock.bSafe[i]) continue;

        CProjectile* p = nullptr;
        CProjectile_Manager::GetInstance()->Spawn(
            Get_PrototypeLevelIndex(), Get_LevelIndex(),
            CProjectile_Rock::POOL_KEY, CProjectile_Rock::PROTOTYPE_TAG, &p);

        if (auto* pRock = static_cast<CProjectile_Rock*>(p))
        {
            pRock->Set_LinkedDecal(m_Rock.pDecals[i]);
            const _float fHeight = ROCK_DROP_HEIGHT + m_pGameInstance_Proxy->RandomFloat(0.f, 20.f);
            pRock->Drop(m_Rock.vTiles[i], fHeight);
        }
    }
}

void CBoss_Metaknight::Set_TopViewCam(_bool bOn)
{
    BOSSCAM_TOPVIEW_DESC d{};
    d.bOn = bOn;
    if (bOn)
    {
        _float3 c{ 0.f, 0.f, 0.f };
        for (int i = 0; i < GIGA_POINT_COUNT; ++i) {
            c.x += s_vGigaPoints[i].x; c.y += s_vGigaPoints[i].y; c.z += s_vGigaPoints[i].z;
        }
        c.x /= GIGA_POINT_COUNT; c.y /= GIGA_POINT_COUNT; c.z /= GIGA_POINT_COUNT;
        d.vCenter = c;
        d.fHeight = TOPVIEW_HEIGHT;
    }
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_TopView, &d);
}

void CBoss_Metaknight::Start_PatternCooldowns(_float fUsedCooldown)
{
    if (s_fRockCooldown <= fUsedCooldown) m_fRockCooldown = s_fRockCooldown;
    if (s_fGigaCooldown <= fUsedCooldown) m_fGigaCooldown = s_fGigaCooldown;
    if (s_fUpperCooldown <= fUsedCooldown) m_fUpperCooldown = s_fUpperCooldown;
}

void CBoss_Metaknight::Enable_CatchBox(_bool bOn)
{
    if (m_pBody)
        m_pBody->Enable_HitBox(CBoss_Metaknight_Body::MKHB_CATCH, bOn);
}

void CBoss_Metaknight::Begin_Demo(EDemo eDemo)
{
    const DEMO_DESC& tDesc = Get_DemoDesc(eDemo);

    Snap_ToDemoOrigin();

    KIRBY_POSITION_SYNC_BEGIN_DESC Sync{};
    Sync.eType = tDesc.eSyncContext;
    XMStoreFloat4x4(&Sync.AnchorWorld,
        Strip_Scale(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
    Sync.fAnimSpeed = s_fDefaultAnimSpeed;
    Sync.fBlendDuration = 0.f;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncBegin, &Sync);

    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return;

    pAnim->Play(tDesc.pClips[0], false, true, 0.f, s_fDefaultAnimSpeed);

    for (_uint i = 1; i < tDesc.iClipCount; ++i)
    {
        CAnimator::ANI_PLAY_INFO tInfo{};
        tInfo.strAniName = tDesc.pClips[i];
        tInfo.bLoop = false;
        tInfo.bRestart = true;
        tInfo.fBlend = 0.f;
        tInfo.fSpeed = s_fDefaultAnimSpeed;
        pAnim->Enqueue(tInfo);
    }
}

void CBoss_Metaknight::End_Demo(EDemo eDemo)
{
    if (eDemo == EDemo::LOCK_LOSE)
        Drop_Galaxia();

    Reanchor_FromRootBone();

    m_pMovement->Set_GravityEnabled(true);

    KIRBY_POSITION_SYNC_END_DESC SyncEnd{ Get_DemoDesc(eDemo).eEndReason };
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncEnd, &SyncEnd);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CBoss_Metaknight::Set_ParryWindow(_bool bOn)
{
    m_Lock.bParryWindow = bOn;

    if (nullptr == m_pHurtBox)
        return;

    CAPSULE_DESC Cap{};
    if (!Get_HurtBoxDesc(Cap))
        return;

    CCollider::COLLIDER_DESC Desc{};
    Desc.pOwner = this;
    Desc.vCenter = Cap.vCenter;
    Desc.fRadius = bOn ? PARRY_HURT_RADIUS : Cap.fRadius;
    Desc.fHeight = Cap.fHeight;
    Desc.vRadians = Cap.vRadians;

    m_pHurtBox->Reset_Bounding(Desc);
}

void CBoss_Metaknight::Begin_LockingSync()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return;

    pAnim->Pause();
    pAnim->Seek(LOCK_GAUGE_START);
    m_Lock.bSyncing = true;
}

void CBoss_Metaknight::Sync_LockingProgress(_float fProgress01)
{
    if (!m_Lock.bSyncing)
        return;

    fProgress01 = fProgress01 < 0.f ? 0.f : (fProgress01 > 1.f ? 1.f : fProgress01);
    m_Lock.fGauge = fProgress01;

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Seek(fProgress01);
}

void CBoss_Metaknight::End_LockingSync()
{
    if (!m_Lock.bSyncing)
        return;

    m_Lock.bSyncing = false;

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Resume();
}

void CBoss_Metaknight::Fire_CutsceneCamera(const _tchar* szTrack)
{
    CUTSCENE_CAMERA_DESC cam{};
    cam.eCam = ECutsceneCam::Cutscene;
    cam.szTrack = szTrack;
    cam.pProgress = Get_BodyAnimator();
    cam.pAnchorWorld = m_pTransformCom->Get_WorldMatrixPtr();
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CBoss_Metaknight::Hide_AllParts()
{
    if (m_pBody)    m_pBody->Set_Active(false);
    if (m_pMant)    m_pMant->Set_Active(false);
    if (m_pSword)   m_pSword->Set_Active(false);
    if (m_pReplica) m_pReplica->Set_Active(false);
}

void CBoss_Metaknight::Update_PhaseTransition(_float fTimeDelta)
{
    if (m_ePhaseTrans == EPhaseTrans::HOP)
    {
        _vector p = m_pTransformCom->Get_State(STATE::POSITION);
        m_fPhaseVelY -= PHASE_HOP_GRAVITY * fTimeDelta;
        _float y = XMVectorGetY(p) + m_fPhaseVelY * fTimeDelta;

        if (m_fPhaseVelY < 0.f && y <= m_fPhaseBaseY)
        {
            m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(p, m_fPhaseBaseY));
            m_pMovement->Sync_To_Controller();
            if (CAnimator* pAnim = Get_BodyAnimator())
                pAnim->Play("DeathLanding", false, true, 0.1f, s_fDefaultAnimSpeed);
            m_ePhaseTrans = EPhaseTrans::LANDING;
            m_pMovement->Set_GravityEnabled(true);
        }
        else
        {
            m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(p, y));
            m_pMovement->Sync_To_Controller();
        }
    }
    else if (m_ePhaseTrans == EPhaseTrans::LANDING)
    {
        CAnimator* pAnim = Get_BodyAnimator();
        if (pAnim && pAnim->Is_Finished())
        {
            pAnim->Play("Wait", false, true, PHASE_WAIT_BLEND, s_fDefaultAnimSpeed);
            m_ePhaseTrans = EPhaseTrans::WAIT;
        }
    }
    else if (m_ePhaseTrans == EPhaseTrans::WAIT)
    {
        CAnimator* pAnim = Get_BodyAnimator();
        if (!pAnim || !pAnim->Is_Blending())
            m_ePhaseTrans = EPhaseTrans::DONE;
    }
}

void CBoss_Metaknight::Update_Attachment()
{
    if (nullptr == m_Lock.pAttachBone || nullptr == m_Lock.pAttachAnchor)
        return;

    _matrix matAttach = XMLoadFloat4x4(m_Lock.pAttachBone) * XMLoadFloat4x4(m_Lock.pAttachAnchor);

    matAttach = XMMatrixRotationY(XMConvertToRadians(ATTACH_YAW_OFFSET)) * matAttach;

    matAttach = Strip_Scale(matAttach);
    matAttach.r[0] *= m_Lock.vSaveScale.x;
    matAttach.r[1] *= m_Lock.vSaveScale.y;
    matAttach.r[2] *= m_Lock.vSaveScale.z;

    m_pTransformCom->Set_WorldMatrix(matAttach);

    if (!m_Lock.bLockFxFired)
    {
        m_Lock.bLockFxFired = true;

        _float3 vPos{}, vLook{};
        XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
        XMStoreFloat3(&vLook,
            XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f)));

        CEffect_Loader::GetInstance()->Spawn(
            L"Meta_Locking", Get_LevelIndex(), vPos, vLook);
    }
}

void CBoss_Metaknight::Enter_Locking()
{
    if (m_Lock.bActive)
        return;

    m_Lock.bActive = true;
    m_Lock.fTimer = 0.f;
    m_Lock.bCamFired = false;
    m_Lock.bJudged = false;
    m_Lock.fGauge = LOCK_GAUGE_START;
    m_Lock.bLockFxFired = false;
    
    m_pGameInstance_Proxy->Publish(EventTag::FullScreen_Flash, nullptr);

    Enable_CatchBox(false);
    Set_ParryWindow(false);

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("LockingSword", false, true, 0.f, s_fDefaultAnimSpeed);

    Begin_LockingSync();
}

void CBoss_Metaknight::Exit_Locking()
{
    if (!m_Lock.bActive)
        return;

    if (m_Lock.bCamFired)
    {
        m_Lock.bCamFired = false;

        m_pGameInstance_Proxy->Publish(EventTag::QTE_Hide, nullptr);

        CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
        m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
    }

    m_Lock.bActive = false;

    if (auto* pBrain = static_cast<CBoss_Brain*>(m_pBrain))
        pBrain->Reset_Tree();
}

void CBoss_Metaknight::Detach_FromKirby()
{
    if (!m_Lock.bAttached)
        return;

    m_Lock.bAttached = false;
    m_Lock.pAttachBone = nullptr;
    m_Lock.pAttachAnchor = nullptr;

    if (m_pController)
        m_pController->Set_Enabled(true);

    m_pMovement->Sync_To_Controller();
    m_pMovement->Set_GravityEnabled(true);
}

void CBoss_Metaknight::Judge_Locking(_bool bPlayerWin)
{
    m_Lock.bJudged = true;

    m_pGameInstance_Proxy->Publish(EventTag::QTE_Hide, nullptr);

    End_LockingSync();

    m_Lock.eOutcome = bPlayerWin ? ELockOutcome::MK_LOSE : ELockOutcome::MK_WIN;

    Detach_FromKirby();
    Exit_Locking();
}

void CBoss_Metaknight::Tick_Cooldowns(_float fTimeDelta)
{
    if (m_fDodgeCooldown > 0.f)
        m_fDodgeCooldown -= fTimeDelta;

    if (Get_Life() != EBOSS_LIFE::ACTIVE)
        return;

    _float* const pCooldowns[] = { &m_fGigaCooldown, &m_fRockCooldown, &m_fUpperCooldown };
    for (_float* pCool : pCooldowns)
        if (*pCool > 0.f)
            *pCool -= fTimeDelta;
}

void CBoss_Metaknight::Tick_Locking(_float fTimeDelta)
{
    if (!m_Lock.bActive)
        return;

    m_Lock.fTimer += fTimeDelta;

    if (!m_Lock.bCamFired && m_Lock.fTimer >= LOCK_CAM_DELAY)
    {
        m_Lock.bCamFired = true;
        Fire_CutsceneCamera(LOCK_CAM_TRACK);
        m_pGameInstance_Proxy->Publish(EventTag::QTE_Show, nullptr);
    }

    if (m_Lock.bCamFired && !m_Lock.bJudged)
    {
        if (m_Lock.fGauge >= LOCK_GAUGE_WIN)       Judge_Locking(true);
        else if (m_Lock.fGauge <= LOCK_GAUGE_LOSE) Judge_Locking(false);
    }

    if (!m_Lock.bJudged && m_Lock.fTimer >= LOCK_TIMEOUT)
    {
        Detach_FromKirby();
        Exit_Locking();
    }
}

void CBoss_Metaknight::Tick_Appear(_float fTimeDelta)
{
    if (m_bAppearPending)
    {
        m_fAppearTimer += fTimeDelta;
        if (m_fAppearTimer >= APPEAR_DELAY)
        {
            m_bAppearPending = false;
            Appear();
        }
    }

    if (Get_Life() == EBOSS_LIFE::INTRO && !s_bSkipIntro)
    {
        CAnimator* pAnim = Get_BodyAnimator();
        if (pAnim && pAnim->Is_Finished())
            m_fIntroHoldTimer += fTimeDelta;
    }
}

void CBoss_Metaknight::Tick_DeathSequence(_float fTimeDelta)
{
    if (m_eDeathStep == EDEATH::NONE || m_eDeathStep == EDEATH::DONE)
        return;

    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
    {
        m_eDeathStep = EDEATH::DONE;
        return;
    }

    switch (m_eDeathStep)
    {
        case EDEATH::POSE_WAIT:
            if (--m_iDeathPoseDelay <= 0)
            {
                pAnim->Pause();
                m_fDeathTimer = DEATH_PAUSE_SEC;

                Play_OneShotSFX(TEXT("CharaBasic_DeadBigEnemy.wav"));

                CAMERA_SHAKE_DESC shake{ 0.8f, DEATH_SHAKE_SEC };
                m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &shake);

                m_eDeathStep = EDEATH::PAUSING;
            }
            break;

        case EDEATH::PAUSING:
            m_fDeathTimer -= fTimeDelta;
            if (m_fDeathTimer <= 0.f)
            {
                pAnim->Resume();

                pAnim->Play("DeathLanding", false, true, 0.f, s_fDefaultAnimSpeed);

                CAnimator::ANI_PLAY_INFO tInfo{};
                tInfo.strAniName = "DeathMaskBreak";
                tInfo.bLoop = false;
                tInfo.bRestart = true;
                tInfo.fBlend = 0.f;
                tInfo.fSpeed = s_fDefaultAnimSpeed;
                pAnim->Enqueue(tInfo);

                m_eDeathStep = EDEATH::MASK;
            }
            break;

        case EDEATH::MASK:
            if (pAnim->Is_Finished())
            {
                m_fDeathTimer = DEATH_MASK_HOLD;
                m_eDeathStep = EDEATH::MASK_HOLD;
            }
            break;

        case EDEATH::MASK_HOLD:
            m_fDeathTimer -= fTimeDelta;
            if (m_fDeathTimer <= 0.f)
            {
                Show_Mant(true);
                Play_MantSync("EscapeStart", false, 0.f, s_fDefaultAnimSpeed);
                Show_EscapeMant(true);
                Play_EscapeMantSequence();
                pAnim->Play("EscapeStart", false, true, 0.f, s_fDefaultAnimSpeed);

                CAnimator::ANI_PLAY_INFO tInfo{};
                tInfo.strAniName = "Escape";
                tInfo.bLoop = false;
                tInfo.bRestart = true;
                tInfo.fBlend = 0.f;
                tInfo.fSpeed = s_fDefaultAnimSpeed;
                pAnim->Enqueue(tInfo);

                m_eDeathStep = EDEATH::ESCAPE;
            }
            break;

        case EDEATH::ESCAPE:
            if (pAnim->Is_Finished())
                m_eDeathStep = EDEATH::DONE;
            break;

        default:
            break;
    }
}

void CBoss_Metaknight::Snap_ToDemoOrigin()
{
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, DEMO_ORIGIN_Y, 0.f, 1.f));
    m_pTransformCom->LookTo(XMVectorSet(0.f, 0.f, -1.f, 0.f));

    if (m_pController)
        m_pController->Set_FootPosition(m_pTransformCom->Get_State(STATE::POSITION));
}

void CBoss_Metaknight::Reanchor_FromRootBone()
{
    if (nullptr == m_pBody)
        return;

    const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr(DEMO_ROOT_BONE);
    if (nullptr == pBone)
        return;

    _matrix matBoneWorld = XMLoadFloat4x4(pBone)
        * XMLoadFloat4x4(m_pBody->Get_CombinedWorldMatrixPtr());

    _vector vNew = XMVectorSetW(matBoneWorld.r[3], 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vNew);
    if (m_pController)
        m_pController->Set_FootPosition(vNew);

    _vector vBoneLook = XMVectorSetY(XMVectorNegate(matBoneWorld.r[2]), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vBoneLook)) > 1e-4f)
        m_pTransformCom->LookTo(XMVector3Normalize(vBoneLook));
}

const CBoss_Metaknight::DEMO_DESC& CBoss_Metaknight::Get_DemoDesc(EDemo eDemo)
{
    static constexpr const _char* UPPER_CUTS[] = {
        "DemoUpperCaliburCut1", "DemoUpperCaliburCut2", "DemoUpperCaliburCut3",
        "DemoUpperCaliburCut4", "DemoUpperCaliburCut5", "DemoUpperCaliburCut6",
        "DemoUpperCaliburCut7",
    };
    static constexpr const _char* LOCKLOSE_CUTS[] = { "DemoLockingSwordWinCut1" };

    static const DEMO_DESC DESCS[] = {
        { KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_UPPERCALIBUR,
          KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_UPPERCALIBUR_END,
          UPPER_CUTS, _countof(UPPER_CUTS) },

        { KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_LOCKING_WIN,
          KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_LOCKING_WIN_END,
          LOCKLOSE_CUTS, _countof(LOCKLOSE_CUTS) },
    };

    return DESCS[static_cast<_uint>(eDemo)];
}

void CBoss_Metaknight::Queue_DeathClips()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return;

    static constexpr const _char* DEATH_CLIPS[] = {
        "DeathLanding", "DeathMaskBreak", "EscapeStart", "Escape",
    };

    pAnim->Play(DEATH_CLIPS[0], false, true, 0.f, s_fDefaultAnimSpeed);

    for (_uint i = 1; i < _countof(DEATH_CLIPS); ++i)
    {
        CAnimator::ANI_PLAY_INFO tInfo{};
        tInfo.strAniName = DEATH_CLIPS[i];
        tInfo.bLoop = false;
        tInfo.bRestart = true;
        tInfo.fBlend = 0.f;
        tInfo.fSpeed = s_fDefaultAnimSpeed;
        pAnim->Enqueue(tInfo);
    }
}

HRESULT CBoss_Metaknight::Ready_MetaEvents()
{
    Subscribe_Event(APPEAR_TAG, [this](void*) {
        if (m_bAppearPending || Get_Life() != EBOSS_LIFE::HIDDEN)
            return;
        m_bAppearPending = true;
        m_fAppearTimer = 0.f;
        Set_Active(true);
        Hide_AllParts();
        });

    Subscribe_Event(EventTag::Enemy_AttachmentBegin, [this](void* pData) {
        const auto* pDesc = static_cast<ENEMY_ATTACHMENT_BEGIN_DESC*>(pData);
        if (nullptr == pDesc || pDesc->eContext != ENEMY_ATTACHMENT_CONTEXT::METAKNIGHT_QTE)
            return;
        if (m_Lock.bAttached)
            return;
        if (nullptr == pDesc->pBoneMatrix || nullptr == pDesc->pAnchorWorld)
            return;

        m_Lock.bAttached = true;
        m_Lock.vSaveScale = m_pTransformCom->Get_Scaled();

        m_Lock.pAttachBone = pDesc->pBoneMatrix;
        m_Lock.pAttachAnchor = pDesc->pAnchorWorld;

        if (m_pController)
            m_pController->Set_Enabled(false);

        m_pMovement->Set_GravityEnabled(false);

        Enter_Locking();
        });

    Subscribe_Event(EventTag::Enemy_AttachmentEnd, [this](void* pData) {
        const auto* pDesc = static_cast<ENEMY_ATTACHMENT_END_DESC*>(pData);
        if (nullptr == pDesc || pDesc->eContext != ENEMY_ATTACHMENT_CONTEXT::METAKNIGHT_QTE)
            return;

        Detach_FromKirby();
        Exit_Locking();
        });

    return S_OK;
}

#ifdef _DEBUG
void CBoss_Metaknight::Debug_TriggerPhaseTransition()
{
    m_bPhaseTransition = true;
    Play_PhaseTransition(1);
}
#endif // _DEBUG

CBoss_Metaknight* CBoss_Metaknight::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Metaknight* pInstance = new CBoss_Metaknight(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBoss_Metaknight");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Metaknight* CBoss_Metaknight::Clone(void* pArg)
{
    CBoss_Metaknight* pInstance = new CBoss_Metaknight(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBoss_Metaknight");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight::Free()
{
    __super::Free();
}

void CBoss_Metaknight::ROCK_FIELD::Build(const _float3 fCornersIn[4])
{
    _float3 c[4] = { fCornersIn[0], fCornersIn[1], fCornersIn[2], fCornersIn[3] };
    Sort_CornersPerimeter(c);

    XMVECTOR c0 = XMLoadFloat3(&c[0]);
    XMVECTOR c1 = XMLoadFloat3(&c[1]);
    XMVECTOR c2 = XMLoadFloat3(&c[2]);
    XMVECTOR c3 = XMLoadFloat3(&c[3]);

    const bool bTileAlongU =
        (XMVectorGetX(XMVector3Length(c1 - c0)) >= XMVectorGetX(XMVector3Length(c3 - c0)));

    const int rowCounts[5] = { 5, 4, 5, 4, 5 };
    int idx = 0;
    for (int r = 0; r < 5; ++r)
    {
        const float rowT = r / 4.f;
        const int   n = rowCounts[r];
        for (int t = 0; t < n; ++t)
        {
            const float tileT = (n == 5) ? (t / 4.f) : ((t + 0.5f) / 4.f);
            const float u = bTileAlongU ? tileT : rowT;
            const float v = bTileAlongU ? rowT : tileT;

            XMVECTOR bottom = XMVectorLerp(c0, c1, u);
            XMVECTOR top = XMVectorLerp(c3, c2, u);
            XMStoreFloat3(&vTiles[idx++], XMVectorLerp(bottom, top, v));
        }
    }
}

void CBoss_Metaknight::ROCK_FIELD::Select_Safe(CGameInstance_Proxy* pProxy)
{
    for (int i = 0; i < TILE_COUNT; ++i)
        bSafe[i] = false;

    int count = 0;
    while (count < SAFE_COUNT)
    {
        const int r = pProxy->RandomInt(0, TILE_COUNT - 1);
        if (!bSafe[r]) { bSafe[r] = true; ++count; }
    }
}
