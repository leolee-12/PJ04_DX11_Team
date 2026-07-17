#include "Boss_Armadillo.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Armadillo_Brain.h"
#include "Boss_Armadillo_Body.h"
#include "Boss_Armadillo_Cage.h"

#include "Projectile_Partner.h"
#include "Projectile_Manager.h"

#include "Effect_Loader.h"

const vector<_float> CBoss_Armadillo::s_Thresholds = {};

CBoss_Armadillo::CBoss_Armadillo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBoss(pDevice, pContext) {
}
CBoss_Armadillo::CBoss_Armadillo(const CBoss_Armadillo& Prototype)
    : CBoss(Prototype) {
}

HRESULT CBoss_Armadillo::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Armadillo::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_strBossName = L"아르마파라파";
    //m_fMaxHP = 1000.f;
    m_fMaxHP = 100.f;
    m_fCurHP = m_fMaxHP;

    m_pMovement->Set_MoveSpeed(4.f);
    m_pMovement->Set_RotSpeed(120.f);

    m_pBody->Get_Animator()->Play("Wait", true, false, 0.f, 1.5f);

    Subscribe_Event(EventTag::QTE_Success, [this](void*) { m_bQTEEscaped = true; });

    Set_Active(true);

    return S_OK;
}

void CBoss_Armadillo::Update(_float fTimeDelta)
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
        m_bDebugWallHit = true;
#endif
    __super::Update(fTimeDelta);
    Tick_DeathSequence(fTimeDelta);
    Update_BodyOffset(fTimeDelta);
    Update_RutTrail(fTimeDelta);
}

void CBoss_Armadillo::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

CMonsterBrain* CBoss_Armadillo::Create_Brain()
{
    return CBoss_Armadillo_Brain::Create(this);
}

void CBoss_Armadillo::Play_Intro()
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Angry", false, true, 0.f, 1.5f);

    m_pController->Set_Solid(false);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);

    BOSSCAM_CONFIG_DESC cfg{};
    cfg.fAimHeight = 4.f;    
    cfg.fShoulderOffset = 0.f;
    m_pGameInstance_Proxy->Publish(EventTag::BossCam_Config, &cfg);
}

_bool CBoss_Armadillo::Is_Intro_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

void CBoss_Armadillo::Play_Death()
{
    Enable_Colliders(false);
    if (auto* p = Get_HitBoxPart())
        p->Enable_AllHitBoxes(false);

    Hide_Cage();
    if (m_pPartner) { m_pPartner->Despawn(); m_pPartner = nullptr; }

    _vector vDir = XMVectorSetY(
        XMLoadFloat3(&Get_BlackBoard().vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        m_pTransformCom->LookTo(XMVector3Normalize(vDir));

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Death", false, true, 0.f, 1.5f);

    m_pGameInstance_Proxy->Publish(EventTag::FullScreen_Flash, nullptr);

    m_bDeathSeq = true;
    m_eDeathStep = EDEATH::POSE_WAIT;
    m_iDeathPoseDelay = 2;
}

_bool CBoss_Armadillo::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return true;
    if (m_bDeathSeq) return false;
    return pAnim->Is_Finished();
}

void CBoss_Armadillo::On_Enter_Corpse()
{
    __super::On_Enter_Corpse();

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

_bool CBoss_Armadillo::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f;
    Out.fHeight = s_fCCT_Height + 0.1f;
    return true;
}

CAnimator* CBoss_Armadillo::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMultiHitBoxPart* CBoss_Armadillo::Get_HitBoxPart() const
{
    return m_pBody;
}

void CBoss_Armadillo::Begin_QTE(_float fSeconds)
{
    m_bQTEEscaped = false;
    m_fQTETimer = fSeconds;
    m_pGameInstance_Proxy->Publish(EventTag::QTE_Show, nullptr);
}

void CBoss_Armadillo::End_QTE()
{
    m_pGameInstance_Proxy->Publish(EventTag::QTE_Hide, nullptr);
}

void CBoss_Armadillo::Fire_Grab()
{
    if (nullptr == m_pBody)
        return;

    KIRBY_ATTACHMENT_BEGIN_DESC grab{};
    grab.pBoneMatrix = m_pBody->Get_BoneMatrixPtr(GRAB_BONE);
    grab.pSourceWorld = m_pTransformCom->Get_WorldMatrixPtr();
    grab.eType = KIRBY_ATTACHMENT_CONTEXT::GORILLA_COMBAT;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_AttachmentBegin, &grab);
}

void CBoss_Armadillo::Fire_Release(KIRBY_ATTACHMENT_END_REASON eType)
{
    KIRBY_ATTACHMENT_END_DESC desc{ eType };
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_AttachmentEnd, &desc);
}

_bool CBoss_Armadillo::Sweep_Wall(const _float3& vDir, _float fDist, _float3* pOutNormal) const
{
#ifdef _DEBUG
    if (m_bDebugWallHit)
    {
        m_bDebugWallHit = false;  
        if (pOutNormal)
        {
            _vector d = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&vDir), 0.f));
            _float fJitter = XMConvertToRadians(((rand() % 81) - 40) * 1.f);
            XMStoreFloat3(pOutNormal, XMVector3Transform(-d, XMMatrixRotationY(fJitter)));
        }
        return true;
    }
#endif

    _float3 vPos;
    XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
    vPos.y += s_fCCT_Height * 0.5f;   // 몸통 중심 높이에서 쏨

    _float3 vNormal{};
    if (!m_pGameInstance_Proxy->Sweep_Sphere(vPos, 2.5f, vDir, fDist, &vNormal))
        return false;

    if (fabsf(vNormal.y) >= 0.5f)     // 바닥/경사는 벽으로 안 침
        return false;

    if (pOutNormal) *pOutNormal = vNormal;
    return true;
}

void CBoss_Armadillo::Set_TwinDanceOffset(_bool bOn)
{
    if (!m_pBody) return;

    if (bOn)
    {
        const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("Partner1L");
        if (!pBone) return;
        m_vBodyOffsetTarget = _float3(-pBone->_41 * 0.5f, 0.f, -pBone->_43 * 0.5f);
    }
    else
        m_vBodyOffsetTarget = _float3(0.f, 0.f, 0.f);

    XMStoreFloat3(&m_vBodyOffsetStart, m_pBody->Get_Transform()->Get_State(STATE::POSITION));
    m_fBodyOffsetT = 0.f;
}

void CBoss_Armadillo::Summon_Partner()
{
    if (m_pPartner) return;

    CProjectile* p = nullptr;
    CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(),
        CProjectile_Partner::POOL_KEY, CProjectile_Partner::PROTOTYPE_TAG, &p);
    if (!p) return;

    p->Attach_To_Socket(m_pBody->Get_BoneMatrixPtr("Partner1L"),
        m_pBody->Get_CombinedWorldMatrixPtr(), XMMatrixIdentity());
    m_pPartner = static_cast<CProjectile_Partner*>(p);
}

void CBoss_Armadillo::Fire_PartnerThrow()
{
    if (!m_pPartner) return;

    _vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vKirby = XMLoadFloat3(&Get_BlackBoard().vTargetPos);

    _vector vDir = XMVectorSetY(vKirby - vSelf, 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        vDir = XMVector3Normalize(vDir);
    else
        vDir = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));

    _vector vStart = vSelf + vDir * (s_fCCT_Radius + 1.5f);

    _float3 vS, vD;
    XMStoreFloat3(&vS, vStart);
    XMStoreFloat3(&vD, vDir);
    m_pPartner->Launch(vS, vD);
    m_pPartner = nullptr;
}

void CBoss_Armadillo::Enable_PartnerSpinHit(_bool b)
{
    if (m_pPartner) m_pPartner->Enable_SpinHitBox(b);
}

void CBoss_Armadillo::Play_PartnerAnim(const _char* szClip, _bool bLoop)
{
    if (m_pPartner) m_pPartner->Play_Anim(szClip, bLoop);
}

void CBoss_Armadillo::Show_Cage()
{
    if (m_pCage)
        m_pCage->Set_Active(true);
}

void CBoss_Armadillo::Hide_Cage()
{
    if (m_pCage)
        m_pCage->Set_Active(false);
}

void CBoss_Armadillo::Set_RollFx(_bool bOn)
{
    if (m_bRutTrail == bOn)
        return;

    m_bRutTrail = bOn;

    if (bOn)
    {
        XMStoreFloat3(&m_vRutLastPos, m_pTransformCom->Get_State(STATE::POSITION));
        m_iRutToggle = 0;

        CEffect_Loader::GetInstance()->Spawn(
            L"RollWind", Get_LevelIndex(),
            _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f), _float3(0.f, 0.f, 0.f),
            m_pTransformCom->Get_WorldMatrixPtr(), &m_pRollWind);
    }
    else
    {
        if (m_pRollWind)
        {
            m_pRollWind->Start_FadeOut(0.3f);
            m_pRollWind = nullptr;      // 핸들은 즉시 놓는다. 아래 주석 참고
        }
    }
}

HRESULT CBoss_Armadillo::Ready_AnimEvents()
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
            case EANIM_EVENT::Projectile:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                if (e.iIntParam == 0) 
                    Summon_Partner();
                else                  
                    Fire_PartnerThrow();
                break;
            }
            case EANIM_EVENT::OnOffPart:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                if (e.iIntParam == 0)
                    Hide_Cage();

                break;
            }
            case EANIM_EVENT::CamTrack:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                if (!e.strParam.empty())
                {
                    wstring w = StrToWstr(e.strParam);
                    Fire_CatchCamera(w.c_str());
                }
                else
                {
                    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
                    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
                }

                break;
            }
        }
        });
    return S_OK;
}

HRESULT CBoss_Armadillo::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Armadillo_Body>(
        CBoss_Armadillo_Body::PROTOTYPE_TAG, CBoss_Armadillo_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;

    m_pCage = Add_MonsterPart<CBoss_Armadillo_Cage>(
        CBoss_Armadillo_Cage::PROTOTYPE_TAG, CBoss_Armadillo_Cage::PART_TAG,
        m_pBody->Get_BoneMatrixPtr("RHaveL"));
    if (!m_pCage) return E_FAIL;

    m_pBody->Set_HitBox_OnEnter(CBoss_Armadillo_Body::AHB_CATCH,
        [this](CCollider* pOther)
        {
            if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
                return;
            if (m_bCatchHit)
                return;

            m_bCatchHit = true;
            m_pBody->Enable_HitBox(CBoss_Armadillo_Body::AHB_CATCH, false);
            Fire_Grab();
        });

    return S_OK;
}

void CBoss_Armadillo::Update_BodyOffset(_float fTimeDelta)
{
    if (!m_pBody || m_fBodyOffsetT >= 1.f)
        return;

    m_fBodyOffsetT = min(m_fBodyOffsetT + fTimeDelta / s_fBodyOffsetBlendTime, 1.f);

    _float t = m_fBodyOffsetT * m_fBodyOffsetT * (3.f - 2.f * m_fBodyOffsetT);

    _vector vPos = XMVectorLerp(XMLoadFloat3(&m_vBodyOffsetStart),
        XMLoadFloat3(&m_vBodyOffsetTarget), t);
    m_pBody->Get_Transform()->Set_State(STATE::POSITION, XMVectorSetW(vPos, 1.f));
}

void CBoss_Armadillo::Fire_CatchCamera(const _tchar* szTrack)
{
    CUTSCENE_CAMERA_DESC cam{};
    cam.eCam = ECutsceneCam::Cutscene;
    cam.szTrack = szTrack;
    cam.pProgress = Get_BodyAnimator();                      
    cam.pAnchorWorld = m_pTransformCom->Get_WorldMatrixPtr();
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CBoss_Armadillo::Tick_DeathSequence(_float fTimeDelta)
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

void CBoss_Armadillo::Update_RutTrail(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (!m_bRutTrail)
        return;

    _vector vNow = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vLast = XMLoadFloat3(&m_vRutLastPos);

    _vector vDelta = XMVectorSetY(vNow - vLast, 0.f);
    _float  fDist = XMVectorGetX(XMVector3Length(vDelta));

    if (fDist < s_fRutInterval)
        return;

    _vector vDir = XMVector3Normalize(vDelta);

    _float3 vLook{};
    XMStoreFloat3(&vLook, vDir);

    _int iCount = static_cast<_int>(fDist / s_fRutInterval);

    for (_int i = 1; i <= iCount; ++i)
    {
        _vector vSpawn = vLast + vDir * (s_fRutInterval * i);
        vSpawn = XMVectorSetY(vSpawn, XMVectorGetY(vNow));

        _float3 vPos{};
        XMStoreFloat3(&vPos, vSpawn);

        CEffect_Loader::GetInstance()->Spawn(
            (m_iRutToggle & 1) ? L"RutB" : L"RutA",
            Get_LevelIndex(), vPos, vLook);

        ++m_iRutToggle;
    }

    XMStoreFloat3(&m_vRutLastPos, vLast + vDir * (s_fRutInterval * iCount));
}

CBoss_Armadillo* CBoss_Armadillo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Armadillo* pInstance = new CBoss_Armadillo(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBoss_Armadillo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Armadillo* CBoss_Armadillo::Clone(void* pArg)
{
    CBoss_Armadillo* pInstance = new CBoss_Armadillo(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBoss_Armadillo");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Armadillo::Free()
{
    __super::Free();
}