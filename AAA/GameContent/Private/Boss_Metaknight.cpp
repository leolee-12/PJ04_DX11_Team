#include "Boss_Metaknight.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Metaknight_Brain.h"
#include "Boss_Metaknight_Body.h"
#include "Boss_Metaknight_ReplicaSword.h"
#include "Boss_Metaknight_Sword.h"
#include "Boss_Metaknight_Mant.h"

#include "AttackDecal.h"

#include "Projectile_Manager.h"
#include "Projectile_MoonShot.h"
#include "Projectile_Rock.h"

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
    if (m_pGameInstance_Proxy->Key_Down(DIK_P))
        Debug_TriggerPhaseTransition();
#endif
    if (m_fDodgeCooldown > 0.f)
        m_fDodgeCooldown -= fTimeDelta;
    if (Get_Life() == EBOSS_LIFE::ACTIVE && m_fGigaCooldown > 0.f)
        m_fGigaCooldown -= fTimeDelta;
    if (Get_Life() == EBOSS_LIFE::ACTIVE && m_fRockCooldown > 0.f)
        m_fRockCooldown -= fTimeDelta;
    if (Get_Life() == EBOSS_LIFE::ACTIVE && m_fUpperCooldown > 0.f)
        m_fUpperCooldown -= fTimeDelta;

    if (m_bLockingQTE)
    {
        m_fLockTimer += fTimeDelta;

        if (!m_bLockCamFired && m_fLockTimer >= LOCK_CAM_DELAY)
        {
            m_bLockCamFired = true;
            Fire_CutsceneCamera(LOCK_CAM_TRACK);
            m_pGameInstance_Proxy->Publish(EventTag::QTE_Show, nullptr);
        }

        if (m_bLockCamFired && !m_bLockJudged)
        {
            if (m_fLockGauge >= LOCK_GAUGE_WIN)
                Judge_Locking(true);
            else if (m_fLockGauge <= LOCK_GAUGE_LOSE)
                Judge_Locking(false);
        }

        if (!m_bLockJudged && m_fLockTimer >= LOCK_TIMEOUT)
        {
            Detach_FromKirby();
            Exit_Locking();
        }
    }

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

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("DeathLanding", false, true, 0.f, 1.f);
}

_bool CBoss_Metaknight::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
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

    constexpr _float fMantBakedScale = 1.f / 1.3f;
    m_pMant->Get_Transform()->Set_Scale(fMantBakedScale, fMantBakedScale, fMantBakedScale);

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
    if (m_bParryWindow && Is_SwordHit(tInfo.eHitType))
    {
        /*METAKNIGHT_PARRY_DESC tParry{};
        tParry.pSourceWorld = m_pTransformCom->Get_WorldMatrixPtr();
        tParry.fAnimSpeed = s_fDefaultAnimSpeed;
        m_pGameInstance_Proxy->Publish(EventTag::Metaknight_ParryBegin, &tParry);*/

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
    if (m_bLockingQTE || m_bAttached)
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
    Build_RockTilePositions(s_vGigaPoints, m_RockTiles);
    Select_SafeTiles();

    _vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
    const _float sx = XMVectorGetX(vSelf), sz = XMVectorGetZ(vSelf);

    for (int i = 0; i < ROCK_TILE_COUNT; ++i)
    {
        if (m_bSafeTile[i])
        {
            if (m_pRockDecals[i]) m_pRockDecals[i]->Set_Active(false);
            continue;
        }

        if (nullptr == m_pRockDecals[i])
        {
            CGameObject* pObj = nullptr;
            if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj,
                m_iPrototypeLevel, CAttackDecal::PROTOTYPE_TAG,
                m_iPrototypeLevel, TEXT("Layer_Effect"), TEXT("RockDecal"), nullptr)))
                continue;
            m_pRockDecals[i] = dynamic_cast<CAttackDecal*>(pObj);
            if (!m_pRockDecals[i]) continue;
        }
        else m_pRockDecals[i]->Set_Active(true);

        _float3 vStart = { sx, m_RockTiles[i].y, sz };
        m_pRockDecals[i]->Place(vStart, ROCK_DECAL_RADIUS, 9999.f);
        m_pRockDecals[i]->Slide_To(m_RockTiles[i], ROCK_SLIDE_TIME);
    }
}

void CBoss_Metaknight::Drop_Rocks()
{
    for (int i = 0; i < ROCK_TILE_COUNT; ++i)
    {
        if (m_bSafeTile[i]) continue;

        CProjectile* p = nullptr;
        CProjectile_Manager::GetInstance()->Spawn(
            Get_PrototypeLevelIndex(), Get_LevelIndex(),
            CProjectile_Rock::POOL_KEY, CProjectile_Rock::PROTOTYPE_TAG, &p);

        if (auto* pRock = static_cast<CProjectile_Rock*>(p))
        {
            pRock->Set_LinkedDecal(m_pRockDecals[i]);
            _float fHeight = ROCK_DROP_HEIGHT + m_pGameInstance_Proxy->RandomFloat(0.f, 20.f);
            pRock->Drop(m_RockTiles[i], fHeight);
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

void CBoss_Metaknight::Begin_UpperCaliburDemo()
{
    static constexpr const _char* CUTS[] = {
        "DemoUpperCaliburCut1", "DemoUpperCaliburCut2", "DemoUpperCaliburCut3",
        "DemoUpperCaliburCut4", "DemoUpperCaliburCut5", "DemoUpperCaliburCut6",
        "DemoUpperCaliburCut7",
    };

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 7.f, 0.f, 1.f));
    m_pTransformCom->LookTo(XMVectorSet(0.f, 0.f, -1.f, 0.f));
    if (m_pController)
        m_pController->Set_FootPosition(m_pTransformCom->Get_State(STATE::POSITION));

    KIRBY_POSITION_SYNC_BEGIN_DESC Sync{};
    Sync.eType = KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_UPPERCALIBUR;
    XMStoreFloat4x4(&Sync.AnchorWorld,
        Strip_Scale(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
    Sync.fAnimSpeed = s_fDefaultAnimSpeed;
    Sync.fBlendDuration = 0.f;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncBegin, &Sync);

    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return;

    pAnim->Play(CUTS[0], false, true, 0.f, s_fDefaultAnimSpeed);

    for (_uint i = 1; i < _countof(CUTS); ++i)
    {
        CAnimator::ANI_PLAY_INFO tInfo{};
        tInfo.strAniName = CUTS[i];
        tInfo.bLoop = false;
        tInfo.bRestart = true;
        tInfo.fBlend = 0.f;
        tInfo.fSpeed = s_fDefaultAnimSpeed;
        pAnim->Enqueue(tInfo);
    }
}

void CBoss_Metaknight::End_UpperCaliburDemo()
{
    if (m_pBody)
    {
        const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("TopL");
        if (pBone)
        {
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
    }

    m_pMovement->Set_GravityEnabled(true);

    KIRBY_POSITION_SYNC_END_DESC SyncEnd{ KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_UPPERCALIBUR_END };
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncEnd, &SyncEnd);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CBoss_Metaknight::Begin_LockLoseDemo()
{
    static constexpr const _char* CUTS[] = {
        "DemoLockingSwordWinCut1"
    };

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 7.f, 0.f, 1.f));
    m_pTransformCom->LookTo(XMVectorSet(0.f, 0.f, -1.f, 0.f));
    if (m_pController)
        m_pController->Set_FootPosition(m_pTransformCom->Get_State(STATE::POSITION));

    KIRBY_POSITION_SYNC_BEGIN_DESC Sync{};
    Sync.eType = KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_UPPERCALIBUR;
    XMStoreFloat4x4(&Sync.AnchorWorld,
        Strip_Scale(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
    Sync.fAnimSpeed = s_fDefaultAnimSpeed;
    Sync.fBlendDuration = 0.f;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncBegin, &Sync);

    CAnimator* pAnim = Get_BodyAnimator();
    if (nullptr == pAnim)
        return;

    pAnim->Play(CUTS[0], false, true, 0.f, s_fDefaultAnimSpeed);

    for (_uint i = 1; i < _countof(CUTS); ++i)
    {
        CAnimator::ANI_PLAY_INFO tInfo{};
        tInfo.strAniName = CUTS[i];
        tInfo.bLoop = false;
        tInfo.bRestart = true;
        tInfo.fBlend = 0.f;
        tInfo.fSpeed = s_fDefaultAnimSpeed;
        pAnim->Enqueue(tInfo);
    }
}

void CBoss_Metaknight::End_LockLoseDemo()
{
    if (m_pBody)
    {
        const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("TopL");
        if (pBone)
        {
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
    }

    m_pMovement->Set_GravityEnabled(true);

    KIRBY_POSITION_SYNC_END_DESC SyncEnd{ KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_UPPERCALIBUR_END };
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncEnd, &SyncEnd);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CBoss_Metaknight::Set_ParryWindow(_bool bOn)
{
    m_bParryWindow = bOn;

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
    m_bLockSyncing = true;
}

void CBoss_Metaknight::Sync_LockingProgress(_float fProgress01)
{
    if (!m_bLockSyncing)
        return;

    fProgress01 = fProgress01 < 0.f ? 0.f : (fProgress01 > 1.f ? 1.f : fProgress01);
    m_fLockGauge = fProgress01;

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Seek(fProgress01);
}

void CBoss_Metaknight::End_LockingSync()
{
    if (!m_bLockSyncing)
        return;

    m_bLockSyncing = false;

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

void CBoss_Metaknight::Build_RockTilePositions(const _float3 fCornersIn[4], _float3 fOutPos[23])
{
    _float3 c[4] = { fCornersIn[0], fCornersIn[1], fCornersIn[2], fCornersIn[3] };
    Sort_CornersPerimeter(c);

    XMVECTOR c0 = XMLoadFloat3(&c[0]);
    XMVECTOR c1 = XMLoadFloat3(&c[1]);
    XMVECTOR c2 = XMLoadFloat3(&c[2]);
    XMVECTOR c3 = XMLoadFloat3(&c[3]);

    float lenA = XMVectorGetX(XMVector3Length(c1 - c0));
    float lenB = XMVectorGetX(XMVector3Length(c3 - c0));

    bool bLongIsU = (lenA >= lenB);
    bool bTileAlongU = bLongIsU;

    const int rowCounts[5] = { 5, 4, 5, 4, 5 };
    int idx = 0;
    for (int r = 0; r < 5; ++r)
    {
        float rowT = r / 4.f;
        int   n = rowCounts[r];
        for (int t = 0; t < n; ++t)
        {
            float tileT = (n == 5) ? (t / 4.f)
                : ((t + 0.5f) / 4.f);

            float u = bTileAlongU ? tileT : rowT;
            float v = bTileAlongU ? rowT : tileT;

            XMVECTOR bottom = XMVectorLerp(c0, c1, u);
            XMVECTOR top = XMVectorLerp(c3, c2, u);
            XMVECTOR p = XMVectorLerp(bottom, top, v);

            XMStoreFloat3(&fOutPos[idx++], p);
        }
    }
}

void CBoss_Metaknight::Select_SafeTiles()
{
    for (int i = 0; i < ROCK_TILE_COUNT; ++i) m_bSafeTile[i] = false;

    int count = 0;
    while (count < ROCK_SAFE_COUNT)
    {
        int r = m_pGameInstance_Proxy->RandomInt(0, ROCK_TILE_COUNT - 1);
        if (!m_bSafeTile[r]) { m_bSafeTile[r] = true; ++count; }
    }
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
    if (nullptr == m_pAttachBone || nullptr == m_pAttachAnchor)
        return;

    _matrix matAttach = XMLoadFloat4x4(m_pAttachBone) * XMLoadFloat4x4(m_pAttachAnchor);

    matAttach = XMMatrixRotationY(XMConvertToRadians(ATTACH_YAW_OFFSET)) * matAttach;

    matAttach = Strip_Scale(matAttach);
    matAttach.r[0] *= m_vAttachSaveScale.x;
    matAttach.r[1] *= m_vAttachSaveScale.y;
    matAttach.r[2] *= m_vAttachSaveScale.z;

    m_pTransformCom->Set_WorldMatrix(matAttach);
}

void CBoss_Metaknight::Enter_Locking()
{
    if (m_bLockingQTE)
        return;

    m_bLockingQTE = true;
    m_fLockTimer = 0.f;
    m_bLockCamFired = false;
    m_bLockJudged = false;
    m_fLockGauge = LOCK_GAUGE_START;

    Enable_CatchBox(false);
    Set_ParryWindow(false);

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("LockingSword", false, true, 0.f, s_fDefaultAnimSpeed);

    Begin_LockingSync();
}

void CBoss_Metaknight::Exit_Locking()
{
    if (!m_bLockingQTE)
        return;

    if (m_bLockCamFired)
    {
        m_bLockCamFired = false;

        m_pGameInstance_Proxy->Publish(EventTag::QTE_Hide, nullptr);

        CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
        m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
    }

    m_bLockingQTE = false;

    if (auto* pBrain = static_cast<CBoss_Brain*>(m_pBrain))
        pBrain->Reset_Tree();
}

void CBoss_Metaknight::Detach_FromKirby()
{
    if (!m_bAttached)
        return;

    m_bAttached = false;
    m_pAttachBone = nullptr;
    m_pAttachAnchor = nullptr;

    if (m_pController)
        m_pController->Set_Enabled(true);

    m_pMovement->Sync_To_Controller();
    m_pMovement->Set_GravityEnabled(true);
}

void CBoss_Metaknight::Judge_Locking(_bool bPlayerWin)
{
    m_bLockJudged = true;

    m_pGameInstance_Proxy->Publish(EventTag::QTE_Hide, nullptr);

    End_LockingSync();

    m_eLockOutcome = bPlayerWin ? ELockOutcome::MK_LOSE : ELockOutcome::MK_WIN;

    Detach_FromKirby();
    Exit_Locking();
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
        if (m_bAttached)
            return;
        if (nullptr == pDesc->pBoneMatrix || nullptr == pDesc->pAnchorWorld)
            return;

        m_bAttached = true;
        m_vAttachSaveScale = m_pTransformCom->Get_Scaled();

        m_pAttachBone = pDesc->pBoneMatrix;
        m_pAttachAnchor = pDesc->pAnchorWorld;

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