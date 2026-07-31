#include "Boss_GorillaRush.h"
#include "Boss_Gorilla_Body.h"
#include "Boss_GorillaRush_Brain.h"
#include "Effect_Loader.h"
#include "GameInstance.h"

const vector<_float> CBoss_GorillaRush::s_Thresholds = {};   // 단일 페이즈

CBoss_GorillaRush::CBoss_GorillaRush(ID3D11Device* d, ID3D11DeviceContext* c) : CBoss(d, c) {}
CBoss_GorillaRush::CBoss_GorillaRush(const CBoss_GorillaRush& p) : CBoss(p) {}

HRESULT CBoss_GorillaRush::Initialize_Prototype() { return S_OK; }

HRESULT CBoss_GorillaRush::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    m_strBossName = L"고르르뭄바";        
    m_fMaxHP = 650.f; 
    m_fCurHP = m_fMaxHP;

    Get_BodyAnimator()->Play("Wait", true, false, 0.f, 1.5f);

    Set_Active(true);
    return S_OK;
}

void CBoss_GorillaRush::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    Debug_KeyInput();
#endif
    __super::Update(fTimeDelta);
    Tick_DeathSequence(fTimeDelta);
}

CAnimator* CBoss_GorillaRush::Get_BodyAnimator() const { return m_pBody ? m_pBody->Get_Animator() : nullptr; }
CMultiHitBoxPart* CBoss_GorillaRush::Get_HitBoxPart() const { return m_pBody; }

CMonsterBrain* CBoss_GorillaRush::Create_Brain() { return CBoss_GorillaRush_Brain::Create(this); }

HRESULT CBoss_GorillaRush::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Gorilla_Body>( 
        CBoss_Gorilla_Body::PROTOTYPE_TAG, CBoss_Gorilla_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;
    return S_OK;
}

HRESULT CBoss_GorillaRush::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        if (Handle_SoundAnimEvent(e, phase))
            return;
        if (Handle_DropStarsAnimEvent(e, phase))
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
                else
                {
                    CAMERA_SHAKE_DESC shake{ 0.25f, 0.f };
                    m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &shake);
                }
                break;
            }
            default:
                break;
        }
        });
    return S_OK;
}

void CBoss_GorillaRush::Play_Intro()
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Roar", false, true, 0.2f, 1.5f);

    m_pController->Set_Solid(false);

    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}
_bool CBoss_GorillaRush::Is_Intro_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
}

void CBoss_GorillaRush::Play_Death()
{
    Enable_Colliders(false);
    if (auto* p = Get_HitBoxPart()) p->Enable_AllHitBoxes(false);

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("DeathDown", false, true, 0.f, 1.5f);

    m_pGameInstance_Proxy->Publish(EventTag::FullScreen_Flash, nullptr);

    m_bDeathSeq = true;
    m_eDeathStep = EDEATH::POSE_WAIT;
    m_iDeathPoseDelay = 2;
}
_bool CBoss_GorillaRush::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return true;
    if (m_bDeathSeq) return false;
    return pAnim->Is_Finished();
}
void CBoss_GorillaRush::Tick_DeathSequence(_float fTimeDelta)
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

void CBoss_GorillaRush::On_Enter_Corpse()
{
    __super::On_Enter_Corpse();
    _vector vPosV = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vLookV = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

    vPosV -= vLookV * 5.f;
    vPosV.m128_f32[1] += 1.f;

    _float3 vPos{};
    XMStoreFloat3(&vPos, vPosV);
    CEffect_Loader::GetInstance()->Spawn(L"DeathSmoke", m_iPrototypeLevel, vPos);


    m_pGameInstance_Proxy->Publish(EventTag::Level_BossDefeated, nullptr);

    CUTSCENE_CAMERA_DESC cam{};
    cam.eCam = ECutsceneCam::Area;
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

_bool CBoss_GorillaRush::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = s_fCCT_Radius + 0.1f; Out.fHeight = s_fCCT_Height + 0.1f; return true;
}

void CBoss_GorillaRush::On_Deserialized() 
{ 
    __super::On_Deserialized(); 
}

#ifdef _DEBUG
void CBoss_GorillaRush::Debug_KeyInput()
{
    if (!m_pGameInstance_Proxy) return;
    if (m_pGameInstance_Proxy->Key_Down(DIK_0)) Appear();
    if (m_pGameInstance_Proxy->Key_Down(DIK_9)) Die();
}
#endif

CBoss_GorillaRush* CBoss_GorillaRush::Create(ID3D11Device* d, ID3D11DeviceContext* c)
{
    auto* p = new CBoss_GorillaRush(d, c);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed: CBoss_GorillaRush"); Safe_Release(p); }
    return p;
}
CBoss_GorillaRush* CBoss_GorillaRush::Clone(void* pArg)
{
    auto* p = new CBoss_GorillaRush(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed Clone: CBoss_GorillaRush"); Safe_Release(p); }
    return p;
}
void CBoss_GorillaRush::Free() { __super::Free(); }