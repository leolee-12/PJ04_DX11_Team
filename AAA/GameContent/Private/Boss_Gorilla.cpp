#include "Boss_Gorilla.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Gorilla_Brain.h"

#include "Boss_Gorilla_Body.h"
#include "Boss_Gorilla_RockHole.h"
#include "Animator.h"

#include "Projectile_Boulder.h"
#include "Projectile_Manager.h"

#include "Effect_Loader.h"

// 3페이즈: 66%, 33% 에서 전환 (PhaseCount = size()+1 = 3) Brain의 Get_PhaseCount와 일치!
const vector<_float> CBoss_Gorilla::s_Thresholds = { 0.5f };

CBoss_Gorilla::CBoss_Gorilla(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CBoss(pDevice, pContext) {
}
CBoss_Gorilla::CBoss_Gorilla(const CBoss_Gorilla& Prototype)
    : CBoss(Prototype) {
}

HRESULT CBoss_Gorilla::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Gorilla::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))   
        return E_FAIL;

    m_strBossName = L"고르르뭄바";
    m_fMaxHP = 1000.f;    
    m_fCurHP = m_fMaxHP;
    m_TraitFlags &= ~MT_BODYCHECK_DAMAGE;

    if (m_pController) 
        m_pController->Set_Solid(false);

    if (m_pMovement)
    {
        m_pMovement->Set_MoveSpeed(3.f);
        m_pMovement->Set_RotSpeed(90.f);
    }

    return S_OK;
}

void CBoss_Gorilla::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        if (m_pMovement) m_pMovement->Sync_To_Controller();
        return;
    }
#endif
    if (m_fFreezeTimer > 0.f)
    {
        m_fFreezeTimer -= fTimeDelta;
        if (m_fFreezeTimer <= 0.f)
        {
            m_fFreezeTimer = 0.f;
            if (CAnimator* pAnim = Get_BodyAnimator()) pAnim->Resume();
        }
    }

    if (m_eLife == EBOSS_LIFE::INTRO)
        Tick_OpeningCatch();

    __super::Update(fTimeDelta);
    Tick_DeathSequence(fTimeDelta);
}

CMonsterBrain* CBoss_Gorilla::Create_Brain()
{
    return CBoss_Gorilla_Brain::Create(this);
}

void CBoss_Gorilla::Play_Intro()
{
    m_iIntroStep = 0;
    m_bIntroDone = false;
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play(s_Intro[0], false, true, 0.2f, 1.5f);

    Fire_Grab();
}

_bool CBoss_Gorilla::Is_Intro_Finished() const
{
    return m_bIntroDone;
}

void CBoss_Gorilla::Play_Death()
{
    Enable_Colliders(false);                                       
    if (auto* p = Get_HitBoxPart())
    {
        p->Enable_AllHitBoxes(false);
    }

    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return;

    pAnim->Play("DeathDown", false, true, 0.f, 1.5f);   // HP0 순간 블렌딩 없이 즉시 컷

    m_pGameInstance_Proxy->Publish(EventTag::FullScreen_Flash, nullptr);

    m_bDeathSeq = true;                                   // 포즈 적용 후 정지하도록 예약
    m_eDeathStep = EDEATH::POSE_WAIT;
    m_iDeathPoseDelay = 2;
}

_bool CBoss_Gorilla::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return true;
    if (m_bDeathSeq) return false;                       
    return pAnim->Is_Finished();
}

void CBoss_Gorilla::On_Enter_Corpse()
{
    __super::On_Enter_Corpse();

    _vector vPosV = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vLookV = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

    vPosV -= vLookV * 5.f;
    vPosV.m128_f32[1] += 1.f;

    _float3 vPos{};
    XMStoreFloat3(&vPos, vPosV);
    CEffect_Loader::GetInstance()->Spawn(L"DeathSmoke", m_iPrototypeLevel, vPos);
}

HRESULT CBoss_Gorilla::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        if (Handle_SharedAnimEvent(e, phase))
            return;

        switch (static_cast<EANIM_EVENT>(e.iEventType))
        {
            case EANIM_EVENT::Fx:
            {
                if (!e.strParam.empty())
                {
                    switch (e.iIntParam)
                    {
                        case 0:
                        {
                            _float3 vPos{};
                            XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
                            CEffect_Loader::GetInstance()->Spawn(StrToWstr(e.strParam), m_iPrototypeLevel, vPos);
                            break;
                        }
                        case 1:
                        {
                            _matrix BoneMatrix = XMLoadFloat4x4(m_pBody->Get_BoneMatrixPtr(THROW_BONE));
                            _matrix Combindmat = XMMatrixMultiply(BoneMatrix, XMLoadFloat4x4(Get_Transform()->Get_WorldMatrixPtr()));
                            Combindmat.r[3].m128_f32[1] = Get_Transform()->Get_State(STATE::POSITION).m128_f32[1];

                            _float3 vPos{};
                            XMStoreFloat3(&vPos, Combindmat.r[3]);
                            CEffect_Loader::GetInstance()->Spawn(StrToWstr(e.strParam), m_iPrototypeLevel, vPos);
                            break;
                        }
                        case 2:
                        {
                            _float3 vPos{}, vLook{};
                            XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
                            XMStoreFloat3(&vLook, m_pTransformCom->Get_State(STATE::LOOK));
                            CEffect_Loader::GetInstance()->Spawn(StrToWstr(e.strParam), m_iPrototypeLevel, vPos, vLook);
                            break;
                        }
                    }
                }
                break;
            }
            case EANIM_EVENT::CamTrack:
            {
                if (e.strParam.empty())
                {
                    if (m_eLife != EBOSS_LIFE::INTRO)
                        break;
                    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
                    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
                }
                else                      
                {
                    if (m_eLife != EBOSS_LIFE::INTRO && (e.strParam == "Roar_camera"))
                        break;
                    wstring w(e.strParam.begin(), e.strParam.end());
                    Fire_CatchCamera(w.c_str());
                }
                break;
            }
            case EANIM_EVENT::CamShake:
            {
                if (e.bIsRange)
                {
                    _float lvl = 0.f;
                    if (phase == ANIM_EVENT_PHASE::BEGIN)
                        lvl = (e.iIntParam > 0 ? e.iIntParam : 50) / 100.f;
                    if (phase == ANIM_EVENT_PHASE::BEGIN || phase == ANIM_EVENT_PHASE::END)
                        m_pGameInstance_Proxy->Publish(EventTag::Camera_Rumble, &lvl);
                }
                break;
            }

            case EANIM_EVENT::FreezeAnim:
            {
                if (m_eLife != EBOSS_LIFE::INTRO)
                    break;
                if (phase == ANIM_EVENT_PHASE::POINT)
                    Begin_AnimFreeze(e.vOffset.x > 0.f ? e.vOffset.x : 3.f);
                break;
            }

            case EANIM_EVENT::PubEvent:
            {
                if (m_eLife != EBOSS_LIFE::INTRO)
                    break;
                if (phase == ANIM_EVENT_PHASE::POINT && !e.strParam.empty())
                {
                    wstring tag(e.strParam.begin(), e.strParam.end());
                    m_pGameInstance_Proxy->Publish(tag, nullptr);
                }
                break;
            }

            case EANIM_EVENT::Projectile:
            {
                if (phase != ANIM_EVENT_PHASE::POINT) break;

                if (e.iIntParam == 0)
                {
                    CProjectile* p = nullptr;
                    const _wstring strKey = e.strParam.empty()
                        ? L"Boulder" : _wstring(e.strParam.begin(), e.strParam.end());
                    CProjectile_Manager::GetInstance()->Spawn(Get_LevelIndex(), strKey,
                        CProjectile_Boulder::PROTOTYPE_TAG, &p);

                    if (p)
                    {
                        p->Attach_To_Socket(m_pBody->Get_BoneMatrixPtr(THROW_BONE),
                            m_pTransformCom->Get_WorldMatrixPtr(), XMMatrixIdentity());
                        m_pHeldRock = p;
                    }

                    _matrix BoneMatrix = XMLoadFloat4x4(m_pBody->Get_BoneMatrixPtr(THROW_BONE));
                    _matrix Combindmat = XMMatrixMultiply(BoneMatrix, XMLoadFloat4x4(Get_Transform()->Get_WorldMatrixPtr()));
                    Combindmat.r[3].m128_f32[1] = Get_Transform()->Get_State(STATE::POSITION).m128_f32[1];
                    m_pRockHole->Active_Effect(Combindmat.r[3]);

                }
                else
                {
                    if (m_pHeldRock)
                    {
                        _vector vHand = m_pHeldRock->Get_Transform()->Get_State(STATE::POSITION);
                        _vector vKirby = XMLoadFloat3(&Get_BlackBoard().vTargetPos);

                        _vector vToKirby = XMVectorSetY(vKirby - vHand, 0.f);
                        if (XMVectorGetX(XMVector3LengthSq(vToKirby)) > 1e-6f)
                            vToKirby = XMVector3Normalize(vToKirby);
                        const _float fFrontOffset = 5.f;
                        _vector vTarget = vKirby - vToKirby * fFrontOffset;

                        _float3 vS, vT; XMStoreFloat3(&vS, vHand); XMStoreFloat3(&vT, vTarget);
                        m_pHeldRock->Launch_Arc(vS, vT, 1.5f, 5.f);
                        m_pHeldRock = nullptr;
                    }
                }
                break;
            }
            default: 
                break;
        }
        });
    return S_OK;
}

void CBoss_Gorilla::Play_PhaseTransition(_int iNewPhase)
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Roar", false, true, 0.2f, 1.5f);
}

_bool CBoss_Gorilla::Is_PhaseTransition_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

void CBoss_Gorilla::On_PhaseChanged(_int iOldPhase, _int iNewPhase)
{
    OutputDebugStringA(("[Gorilla] Phase " + std::to_string(iOldPhase)
        + " -> " + std::to_string(iNewPhase) + "\n").c_str());
    // TODO: 전환 시 transient 상태 리셋(가드/속도 등) + 셰이더 파라미터 갱신
}

_bool CBoss_Gorilla::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f;
    Out.fHeight = s_fCCT_Height + 0.1f;
    return true;
}

CAnimator* CBoss_Gorilla::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMultiHitBoxPart* CBoss_Gorilla::Get_HitBoxPart() const
{
    return m_pBody;
}

HRESULT CBoss_Gorilla::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Gorilla_Body>(
        CBoss_Gorilla_Body::PROTOTYPE_TAG, CBoss_Gorilla_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;

    Add_PartObject(m_iPrototypeLevel, CBoss_Gorilla_RockHole::PROTOTYPE_TAG, CBoss_Gorilla_RockHole::PART_TAG);
    m_pRockHole = dynamic_cast<CBoss_Gorilla_RockHole*>(m_PartObjects[CBoss_Gorilla_RockHole::PART_TAG]);
    if (!m_pRockHole)
        return E_FAIL;

    return S_OK;
}

void CBoss_Gorilla::Tick_OpeningCatch()
{
    if (m_bIntroDone) return;

    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) { m_bIntroDone = true; return; }
    if (!pAnim->Is_Finished()) return;          // 현재 클립 재생 중

    ++m_iIntroStep;
    constexpr _int iCount = static_cast<_int>(sizeof(s_Intro) / sizeof(s_Intro[0]));
    if (m_iIntroStep >= iCount)
    {
        m_bIntroDone = true;                    // Roar 끝 -> INTRO 종료 -> 베이스가 네임플레이트 publish
        // 안전망: BT 시작 전 게임플레이 캠 보장(이미 복귀돼 있으면 무해).
        CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
        m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
        return;
    }
    pAnim->Play(s_Intro[m_iIntroStep], false, true, 0.2f, 1.5f);
}

void CBoss_Gorilla::Fire_CatchCamera(const _tchar* szTrack)
{
    CUTSCENE_CAMERA_DESC cam{};
    cam.eCam = ECutsceneCam::Cutscene;
    cam.szTrack = szTrack;
    cam.pProgress = Get_BodyAnimator();                      // 현재 잡기 클립 진행도
    cam.pAnchorWorld = m_pTransformCom->Get_WorldMatrixPtr();   // 전투 고릴라 월드(라이브)
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CBoss_Gorilla::Fire_Grab()
{
    if (nullptr == m_pBody)
        return;

    CUTSCENE_GRAB_DESC grab{};
    grab.pBoneMatrix = m_pBody->Get_BoneMatrixPtr(GRAB_BONE);          
    grab.pSourceWorld = m_pTransformCom->Get_WorldMatrixPtr();         
    grab.eType = GRAB_TYPE::GORILLA_COMBAT;
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_GrabKirby, &grab);
}

void CBoss_Gorilla::Tick_DeathSequence(_float fTimeDelta)
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

                m_pGameInstance_Proxy->Stop_BGM();
                Play_OneShotSFX(
                    TEXT("CharaBasic_DeadBigEnemy.wav"));

                CAMERA_SHAKE_DESC shake{ 0.8f, DEATH_SHAKE_SEC }; 
                m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &shake);
                m_eDeathStep = EDEATH::PAUSING;
            }
            break;

        case EDEATH::PAUSING:
            m_fDeathPauseTimer -= fTimeDelta;
            if (m_fDeathPauseTimer <= 0.f)
            {
                pAnim->Resume();                            // 애님 재생, DeathDamage 다음 DeathDown까지
                m_eDeathStep = EDEATH::PLAYING;
                m_bDeathSeq = false;                        // 이후 Is_Death_Finished에 위임
            }
            break;
        default: break;
    }
}

void CBoss_Gorilla::Begin_AnimFreeze(_float fSeconds)
{
    if (CAnimator* pAnim = Get_BodyAnimator()) pAnim->Pause();
    m_fFreezeTimer = fSeconds;
}

CBoss_Gorilla* CBoss_Gorilla::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Gorilla* pInstance = new CBoss_Gorilla(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBoss_Gorilla");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Gorilla* CBoss_Gorilla::Clone(void* pArg)
{
    CBoss_Gorilla* pInstance = new CBoss_Gorilla(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBoss_Gorilla");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Gorilla::Free()
{
    __super::Free();
}