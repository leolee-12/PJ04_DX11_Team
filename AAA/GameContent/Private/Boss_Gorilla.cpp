#include "Boss_Gorilla.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Gorilla_Brain.h"

#include "Boss_Gorilla_Body.h"
#include "Animator.h"

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

    m_strBossName = L"고릴라 보스";
    m_fMaxHP = 1000.f;    
    m_fCurHP = 400.f;

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

HRESULT CBoss_Gorilla::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        switch (static_cast<EANIM_EVENT>(e.iEventType))
        {
            case EANIM_EVENT::CamTrack:
            {
                if (e.strParam.empty())
                {
                    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
                    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
                }
                else                      
                {
                    wstring w(e.strParam.begin(), e.strParam.end());
                    Fire_CatchCamera(w.c_str());
                }
                break;
            }
            case EANIM_EVENT::CamShake:
                if (e.bIsRange)
                {
                    _float lvl = 0.f;
                    if (phase == ANIM_EVENT_PHASE::BEGIN)
                        lvl = (e.iIntParam > 0 ? e.iIntParam : 50) / 100.f;
                    if (phase == ANIM_EVENT_PHASE::BEGIN || phase == ANIM_EVENT_PHASE::END)
                        m_pGameInstance_Proxy->Publish(EventTag::Camera_Rumble, &lvl);
                }
                break;

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

            default: break;
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
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_GrabKirby, &grab);
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