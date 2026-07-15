#include "Boss_Leopard.h"
#include "GameInstance.h"
#include "Boss_Leopard_Body.h"
#include "Animator.h"
#include "Effect_Loader.h"
#include "Boss_Leopard_Brain.h"
#include "GameContent_AnimEvents.h"

const vector<_float> CBoss_Leopard::s_Thresholds = {};

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

    Set_Active(true);
    return S_OK;
}

void CBoss_Leopard::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
    Tick_DeathSequence(fTimeDelta);
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
        if (Handle_SharedAnimEvent(e, phase))
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

            default: break;
        }
        });
    return S_OK;
}

CMonsterBrain* CBoss_Leopard::Create_Brain()
{
    return CBoss_Leopard_Brain::Create(this);
}

void CBoss_Leopard::Play_Intro()
{
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Angry", false, true, 0.f, 1.5f);   // TODO: 레오파드 등장 클립

    BOSSCAM_CONFIG_DESC cfg{};
    cfg.fAimHeight = 4.f;
    cfg.fShoulderOffset = 0.f;
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

    // 커비를 바라보고 죽음 (수평만, LookTo는 이제 w-safe)
    _vector vDir = XMVectorSetY(
        XMLoadFloat3(&Get_BlackBoard().vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
        m_pTransformCom->LookTo(XMVector3Normalize(vDir));

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("DeathDamage", false, true, 0.f, 1.f);   // TODO: 레오파드 Death 클립

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
void CBoss_Leopard::Free() { __super::Free(); }