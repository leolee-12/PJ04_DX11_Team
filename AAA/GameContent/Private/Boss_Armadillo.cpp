#include "Boss_Armadillo.h"
#include "GameInstance.h"
#include "Monster_Movement.h"
#include "Boss_Armadillo_Brain.h"
#include "Boss_Armadillo_Body.h"
#include "Animator.h"

// 페이즈 임계값. 비어 있으면 1페이즈 (Brain의 Get_PhaseCount = size()+1 과 일치해야 함)
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

    m_strBossName = L"아르마딜로";      // TODO: 정식 이름
    m_fMaxHP = 800.f;
    m_fCurHP = m_fMaxHP;

    if (m_pMovement)
    {
        m_pMovement->Set_MoveSpeed(4.f);
        m_pMovement->Set_RotSpeed(120.f);
    }

    //임시
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
#endif
    __super::Update(fTimeDelta);
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
    // TODO: 인트로 연출 확정 시 교체 (지금은 클립 하나 재생하고 끝)
    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("Wait", false, true, 0.2f, 1.f);
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

    if (CAnimator* pAnim = Get_BodyAnimator())
        pAnim->Play("DeathDown", false, true, 0.f, 1.f);   // TODO: 사망 클립명 확인
}

_bool CBoss_Armadillo::Is_Death_Finished() const
{
    CAnimator* pAnim = Get_BodyAnimator();
    return pAnim ? pAnim->Is_Finished() : true;
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

HRESULT CBoss_Armadillo::Ready_AnimEvents()
{
    CAnimator* pAnim = Get_BodyAnimator();
    if (!pAnim) return E_FAIL;

    pAnim->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase) {
        if (Handle_SharedAnimEvent(e, phase))
            return;
        // TODO: 아르마딜로 전용 이벤트(Fx, CamShake, Projectile 등) 채우기
        });
    return S_OK;
}

HRESULT CBoss_Armadillo::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CBoss_Armadillo_Body>(
        CBoss_Armadillo_Body::PROTOTYPE_TAG, CBoss_Armadillo_Body::PART_TAG);
    if (!m_pBody) return E_FAIL;

    return S_OK;
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