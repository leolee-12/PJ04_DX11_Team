#include "Boss_Leopard.h"
#include "Boss_Leopard_Body.h"
#include "Effect_Loader.h"
#include "Boss_Leopard_Brain.h"
#include "GameContent_AnimEvents.h"

#include "Projectile_Nail.h"
#include "Projectile_Manager.h"

#include "Monster_Movement.h"

#include "Spotlight_Rig.h"

const vector<_float> CBoss_Leopard::s_Thresholds = {};

const _float3 CBoss_Leopard::s_vPillarPos[CBoss_Leopard::PILLAR_COUNT] = {
    /* FL */ { 706.5f, 89.852f, 183.79f },
    /* FR */ { 706.4f, 89.852f, 219.8f },
    /* BR */ { 742.2f, 89.852f, 219.8f },
    /* BL */ { 742.3f, 89.852f, 184.f },
};

CBoss_Leopard::CBoss_Leopard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBoss(pDevice, pContext) {
}
CBoss_Leopard::CBoss_Leopard(const CBoss_Leopard& Prototype)
    : CBoss(Prototype) {
}

HRESULT CBoss_Leopard::Initialize_Prototype() { return S_OK; }

HRESULT CBoss_Leopard::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;

    m_strBossName = L"캐롤라인";
    m_fMaxHP = 100.f;
    m_fCurHP = m_fMaxHP;

    m_pBody->Get_Animator()->Play("Wait", true, false, 0.f, 1.5f);

    Set_Active(true);
    return S_OK;
}

void CBoss_Leopard::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    Debug_KeyInput();
#endif

    __super::Update(fTimeDelta);
    Tick_DeathSequence(fTimeDelta);
    Update_Afterimage(fTimeDelta);

    if (nullptr == m_pSpotRig)
        Ready_TestSpotlight();          

    if (m_pSpotRig)
    {
        switch (m_eSpotTarget)
        {
            case SPOT_LEOPARD:
                m_pSpotRig->Set_Target(m_pTransformCom->Get_State(STATE::POSITION));
                break;
            case SPOT_KIRBY:
                m_pSpotRig->Set_Target(XMLoadFloat3(&Get_BlackBoard().vTargetPos));
                break;
            case SPOT_FIXED:
                break;
        }
        m_pSpotRig->Update(fTimeDelta);
    }
}

CAnimator* CBoss_Leopard::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}
CMultiHitBoxPart* CBoss_Leopard::Get_HitBoxPart() const { return m_pBody; }

HRESULT CBoss_Leopard::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Leopard_Body>(
        CBoss_Leopard_Body::PROTOTYPE_TAG, CBoss_Leopard_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;
    return S_OK;
}

HRESULT CBoss_Leopard::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        if (Handle_SoundAnimEvent(e, phase))
            return;
        if (Handle_FxAnimEvent(e, phase))
            return;

        if (phase != ANIM_EVENT_PHASE::POINT)
            return;
        if (!m_pBody)
            return;
        switch (static_cast<EANIM_EVENT>(e.iEventType))
        {
            case EANIM_EVENT::SetEye:    // iIntParam: 0=Angry 1=Normal 2=Open 3=Close
                m_pBody->Set_EyeState(static_cast<CBoss_Leopard_Body::EYE>(e.iIntParam));
                break;

            case EANIM_EVENT::OnOffMesh:   // iIntParam: 0=Normal 1=Attack
                m_pBody->Set_ClawState(static_cast<CBoss_Leopard_Body::CLAW>(e.iIntParam));
                break;

            case EANIM_EVENT::Projectile:      // iIntParam: 0=생성/부착, 1=발사(다음 단계)
                if (e.iIntParam == 0) Spawn_HandNails();
                else                  Launch_NextHandNail();
                break;

            default: break;
        }
        });
    return S_OK;
}

void CBoss_Leopard::On_Deserialized()
{
    __super::On_Deserialized();
    m_pTransformCom->Set_Scale(1.5f, 1.5f, 1.5f);
}

_int CBoss_Leopard::Advance_ToAdjacentPillar()
{
    const _int iDir = m_pGameInstance_Proxy->RandomInt(0, 1) ? +1 : -1;
    m_iCurPillar = (m_iCurPillar + iDir + PILLAR_COUNT) % PILLAR_COUNT;
    return iDir;
}

void CBoss_Leopard::Spawn_HandNails()
{
    if (!m_pBody) return;

    const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("RHaveL");
    const _float4x4* pOwnerWorld = m_pBody->Get_CombinedWorldMatrixPtr();
    if (!pBone || !pOwnerWorld) return;

    const _float fTotalDeg = 80.f;   
    for (_int i = 0; i < NAIL_COUNT; ++i)
    {
        if (m_pNails[i]) continue;   

        CProjectile* p = nullptr;
        CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(),
            CProjectile_Nail::POOL_KEY, CProjectile_Nail::PROTOTYPE_TAG, &p);
        if (!p) continue;

        _float t = (NAIL_COUNT == 1) ? 0.f : (_float)i / (NAIL_COUNT - 1);   
        _float deg = -fTotalDeg * 0.5f + fTotalDeg * t;                      
        _matrix matOffset = XMMatrixRotationY(XMConvertToRadians(deg + 90.f));      

        p->Attach_To_Socket(pBone, pOwnerWorld, matOffset);
        m_pNails[i] = static_cast<CProjectile_Nail*>(p);
    }
}

void CBoss_Leopard::Launch_NextHandNail()
{
    const _float3 vTarget = Get_BlackBoard().vTargetPos;   

    for (_int i = 0; i < NAIL_COUNT; ++i)
    {
        if (!m_pNails[i]) continue;
        m_pNails[i]->Launch_At(vTarget);
        m_pNails[i] = nullptr;
        return;                   
    }
}

void CBoss_Leopard::Enter_PillarMode()
{
    if (m_bPillarMode) return;
    m_bPillarMode = true;

    BOSSCAM_CONFIG_DESC cfg{};
    cfg.fAimHeight = -3.f;
    cfg.fShoulderOffset = 0.f;
    cfg.fHeight = 2.f;
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_Config, &cfg);

    Enable_Controller(false);
    if (m_pMovement)
    {
        m_pMovement->Set_GravityEnabled(false);
        m_pMovement->Sync_To_Controller();
    }
}

void CBoss_Leopard::Exit_PillarMode()
{
    if (!m_bPillarMode) return;
    m_bPillarMode = false;

    BOSSCAM_CONFIG_DESC cfg{};
    cfg.fAimHeight = 3.f;
    cfg.fShoulderOffset = 0.f;
    cfg.fHeight = 2.f;
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_Config, &cfg);

    Enable_Controller(true);
    if (m_pMovement)
    {
        m_pMovement->Set_GravityEnabled(true);
        m_pMovement->Sync_To_Controller();       
    }
}

void CBoss_Leopard::Set_AfterimageFx(_bool bOn, const _wstring& strEffectId)
{
    if (m_bAfterimageFx == bOn)
        return;
    m_bAfterimageFx = bOn;

    if (bOn)
    {
        m_strAfterimageId = strEffectId;   
        m_fAfterimageAccum = 0.f;
        Spawn_Afterimage();                 
    }
}

void CBoss_Leopard::Spawn_FloorFx()
{
    _float3 vPos{}, vLook{};
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
    XMStoreFloat3(&vLook,
        XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f)));

    CEffect_Loader::GetInstance()->Spawn(
        L"Leopard_Floor", Get_LevelIndex(), vPos, vLook);
}

void CBoss_Leopard::Spawn_JumpSmoke(const _float3& vPos)
{
    _float3 vLook{};
    XMStoreFloat3(&vLook,
        XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f)));

    CEffect_Loader::GetInstance()->Spawn(
        L"LeoJump_Smoke", Get_LevelIndex(), vPos, vLook);
}

void CBoss_Leopard::Spotlight_Off()
{
    if (!m_pSpotRig) return;
    m_eSpotTarget = SPOT_LEOPARD;
    m_pSpotRig->Set_Enabled(false);
    m_pSpotRig->Push();
}

void CBoss_Leopard::Spotlight_On_Snap()
{
    if (!m_pSpotRig) return;
    m_eSpotTarget = SPOT_LEOPARD;
    m_pSpotRig->Set_Target(m_pTransformCom->Get_State(STATE::POSITION));
    m_pSpotRig->Snap();
    m_pSpotRig->Set_Enabled(true);
    m_pSpotRig->Push();
}

void CBoss_Leopard::Spotlight_TrackKirby()
{
    m_eSpotTarget = SPOT_KIRBY;
}

void CBoss_Leopard::Spotlight_LockTarget(_fvector vWorldPos)
{
    if (!m_pSpotRig) return;
    m_eSpotTarget = SPOT_FIXED;
    m_pSpotRig->Set_Target(vWorldPos);
}

CMonsterBrain* CBoss_Leopard::Create_Brain()
{
    return CBoss_Leopard_Brain::Create(this);
}

void CBoss_Leopard::Play_Intro()
{
    Spotlight_Off();
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Anger", false, true, 0.f, BASE_ANIM_SPEED);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);

    BOSSCAM_CONFIG_DESC cfg{};
    cfg.fAimHeight = 3.f;
    cfg.fShoulderOffset = 0.f;
    cfg.fHeight = 2.f;
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_Config, &cfg);
}

_bool CBoss_Leopard::Is_Intro_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

void CBoss_Leopard::Play_Death()
{
    Enable_Colliders(false);
    if (auto* p = Get_HitBoxPart())
        p->Enable_AllHitBoxes(false);

    Spotlight_Off();

    _vector vDir = XMVectorSetY(
        XMLoadFloat3(&Get_BlackBoard().vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        m_pTransformCom->LookTo(XMVector3Normalize(vDir));

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Death", false, true, 0.f, BASE_ANIM_SPEED);

    m_pGameInstance_Proxy->Publish(EventTag::FullScreen_Flash, nullptr);

    m_bDeathSeq = true;
    m_eDeathStep = EDEATH::POSE_WAIT;
    m_iDeathPoseDelay = 2;
}

_bool CBoss_Leopard::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return true;
    if (m_bDeathSeq) return false;
    return pAnim->Is_Finished();
}

void CBoss_Leopard::Tick_DeathSequence(_float fTimeDelta)
{
    if (!m_bDeathSeq) return;
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) { m_bDeathSeq = false; return; }

    switch (m_eDeathStep)
    {
        case EDEATH::POSE_WAIT:
            if (--m_iDeathPoseDelay <= 0)
            {
                pAnim->Pause();
                m_fDeathPauseTimer = DEATH_PAUSE_SEC;
                Play_OneShotSFX(TEXT("CharaBasic_DeadBigEnemy.wav"));
                CAMERA_SHAKE_DESC shake{ 0.8f, DEATH_SHAKE_SEC };
                m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &shake);
                m_eDeathStep = EDEATH::PAUSING;
            }
            break;
        case EDEATH::PAUSING:
            m_fDeathPauseTimer -= fTimeDelta;
            if (m_fDeathPauseTimer <= 0.f)
            {
                pAnim->Resume();
                m_eDeathStep = EDEATH::PLAYING;
                m_bDeathSeq = false;
            }
            break;
        default: break;
    }
}

void CBoss_Leopard::Update_Afterimage(_float fTimeDelta)
{
    if (!m_bAfterimageFx)
        return;

    m_fAfterimageAccum += fTimeDelta;
    while (m_fAfterimageAccum >= s_fAfterimageInterval)
    {
        m_fAfterimageAccum -= s_fAfterimageInterval;
        Spawn_Afterimage();
    }
}

void CBoss_Leopard::Spawn_Afterimage()
{
    if (m_strAfterimageId.empty())
        return;

    _float3 vPos{}, vLook{};
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
    XMStoreFloat3(&vLook,
        XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f)));

    CEffect_Loader::GetInstance()->Spawn(
        m_strAfterimageId, Get_LevelIndex(), vPos, vLook);
}

HRESULT CBoss_Leopard::Ready_TestSpotlight()
{
    LIGHT_DESC tInit{};
    tInit.eType = LIGHT::SPOT;
    tInit.vSpecular = _float4(1.f, 1.f, 0.25f, 1.f);
    tInit.vAmbient = _float4(0.f, 0.f, 0.f, 1.f);
    tInit.fRange = 90.f;

    m_pSpotRig = CSpotlight_Rig::Create(m_pGameInstance_Proxy, tInit);
    if (nullptr == m_pSpotRig)
        return E_FAIL;

    m_pSpotRig->Set_Color(_float3(1.f, 1.f, 0.25f), 15.f);
    m_pSpotRig->Set_Cone(20.f, 25.f);
    m_pSpotRig->Set_FollowSpeed(6.f);

    _vector vSpawnPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vLightPos = vSpawnPos + XMVectorSet(0.f, 30.f, 0.f, 0.f);
    m_pSpotRig->Set_Position(vLightPos);
    m_pSpotRig->Set_Target(vSpawnPos);
    m_pSpotRig->Snap_Fade(true);
    m_pSpotRig->Push();
    return S_OK;
}

#ifdef _DEBUG
void CBoss_Leopard::Debug_KeyInput()
{
    if (m_pGameInstance_Proxy->Key_Down(DIK_0))
        Appear();
    if (m_pGameInstance_Proxy->Key_Down(DIK_9)) 
        Die();
}
#endif

void CBoss_Leopard::On_Enter_Corpse()
{
    __super::On_Enter_Corpse();   // Boss_Died publish

    _vector vPosV = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vLookV = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
    vPosV -= vLookV * 2.5f;
    vPosV = XMVectorSetY(vPosV, XMVectorGetY(vPosV) + 1.f);

    _float3 vPos{};
    XMStoreFloat3(&vPos, vPosV);
    CEffect_Loader::GetInstance()->Spawn(L"DeathSmoke", m_iPrototypeLevel, vPos);

    m_pGameInstance_Proxy->Publish(EventTag::Level_BossDefeated, nullptr);

    CUTSCENE_CAMERA_DESC cam{};
    cam.eCam = ECutsceneCam::Area;
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

_bool CBoss_Leopard::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f;
    Out.fHeight = s_fCCT_Height + 0.1f;
    return true;
}

CBoss_Leopard* CBoss_Leopard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Leopard* p = new CBoss_Leopard(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CBoss_Leopard"); Safe_Release(p); }
    return p;
}
CBoss_Leopard* CBoss_Leopard::Clone(void* pArg)
{
    CBoss_Leopard* p = new CBoss_Leopard(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CBoss_Leopard"); Safe_Release(p); }
    return p;
}
void CBoss_Leopard::Free()
{ 
    if (m_pSpotRig) { m_pSpotRig->Set_Enabled(false); m_pSpotRig->Push(); }
    Safe_Release(m_pSpotRig);

    __super::Free(); 
}