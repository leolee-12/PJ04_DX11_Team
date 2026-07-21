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

const _float3 CBoss_Metaknight::s_vGigaPoints[CBoss_Metaknight::GIGA_POINT_COUNT] = {
    { 20.5f, 7.23f, 15.f },
    { 20.5f, 7.23f, -15.f },
    { -20.5f, 7.23f, 15.f },
    { -20.5f, 7.23f, -15.f },
};

const vector<_float> CBoss_Metaknight::s_Thresholds = {};

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

    Subscribe_Event(APPEAR_TAG, [this](void*) {
        if (m_bAppearPending || Get_Life() != EBOSS_LIFE::HIDDEN)
            return;
        m_bAppearPending = true;
        m_fAppearTimer = 0.f;
        Set_Active(true);
        Hide_AllParts();
        });

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
        Request_RockDrop();
#endif
    if (m_fDodgeCooldown > 0.f)
        m_fDodgeCooldown -= fTimeDelta;
    if (Get_Life() == EBOSS_LIFE::ACTIVE && m_fGigaCooldown > 0.f)
        m_fGigaCooldown -= fTimeDelta;

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
}

void CBoss_Metaknight::Late_Update(_float fTimeDelta)
{
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

    m_pReplica = Add_MonsterPart<CBoss_Metaknight_ReplicaSword>(
        CBoss_Metaknight_ReplicaSword::PROTOTYPE_TAG, CBoss_Metaknight_ReplicaSword::PART_TAG,
        m_pBody->Get_BoneMatrixPtr("RHaveL"));
    if (!m_pReplica) return E_FAIL;

    m_pMant = Add_MonsterPart<CBoss_Metaknight_Mant>(
        CBoss_Metaknight_Mant::PROTOTYPE_TAG, CBoss_Metaknight_Mant::PART_TAG);
    if (!m_pMant) return E_FAIL;

    constexpr _float fMantBakedScale = 1.f / 1.3f;
    m_pMant->Get_Transform()->Set_Scale(fMantBakedScale, fMantBakedScale, fMantBakedScale);

    Set_ActiveSword(EMK_SWORD::GALAXIA);

    return S_OK;
}

const _float4x4* CBoss_Metaknight::Get_FxParentMatrix(const _wstring& strFx) const
{
    return m_pTransformCom->Get_WorldMatrixPtr();
}

void CBoss_Metaknight::Damaged(const ATTACK_INFO& tInfo)
{
    if (m_bDodgeInvuln)
        return;

    if (!m_bAttackBusy && m_fDodgeCooldown <= 0.f)
    {
        m_bDodgeRequested = true;
        return;
    }

    __super::Damaged(tInfo);
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

    _vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
    const _float sx = XMVectorGetX(vSelf), sz = XMVectorGetZ(vSelf);

    for (int i = 0; i < ROCK_TILE_COUNT; ++i)
    {
        CGameObject* pObj = nullptr;
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj,
            m_iPrototypeLevel, CAttackDecal::PROTOTYPE_TAG,
            m_iPrototypeLevel, TEXT("Layer_Effect"), TEXT("RockDecal"), nullptr)))
            continue;

        auto* pDecal = dynamic_cast<CAttackDecal*>(pObj);
        if (!pDecal) continue;

        _float3 vStart = { sx, m_RockTiles[i].y, sz };
        pDecal->Place(vStart, ROCK_DECAL_RADIUS, 9999.f);
        pDecal->Slide_To(m_RockTiles[i], ROCK_SLIDE_TIME);
        m_pRockDecals[i] = pDecal;
    }
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

void CBoss_Metaknight::Test_SpawnRockDecals()
{
    _float3 tiles[23];
    Build_RockTilePositions(s_vGigaPoints, tiles);

    const _float fRadius = 5.f;

    for (int i = 0; i < 23; ++i)
    {
        CGameObject* pObj = nullptr;
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj,
            m_iPrototypeLevel, CAttackDecal::PROTOTYPE_TAG,
            m_iPrototypeLevel, TEXT("Layer_Effect"), TEXT("RockDecal_Test"),
            nullptr)))
            continue;

        if (auto* pDecal = dynamic_cast<CAttackDecal*>(pObj))
            pDecal->Place(tiles[i], fRadius, 9999.f);   // 오래 유지
    }
}

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