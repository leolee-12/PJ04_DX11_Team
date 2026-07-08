#include "Kabu.h"
#include "GameInstance.h"
#include "Monster_RailMovement.h"
#include "Animator.h"
#include "Monster_StateMachine.h"

#include "Kabu_Body.h"
#include "Kabu_Brain.h"

#include "Kabu_State_Idle.h"
#include "Kabu_State_WarpIn.h"
#include "Kabu_State_WarpOut.h"

#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"


CKabu::CKabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CKabu::CKabu(const CKabu& Prototype)
    : CMonster (Prototype)
{
}

HRESULT CKabu::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKabu::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_eCopyAbility = COPY_ABILITY_TYPE::NONE;

    return S_OK;
}

_bool CKabu::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { 0.75f };
    Out.fHeight = { 0.40f };

    return true;
}

CAnimator* CKabu::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

void CKabu::Set_RailDesc(const LD_RAIL_DESC& Desc)
{
    if (nullptr == m_pMovement)
        return;

    static_cast<CMonster_RailMovement*>(m_pMovement)->Set_Rail(Desc);
}

void  CKabu::Set_Visible(_bool bVisible)
{
    m_bVisible = bVisible;
    if (m_pBody)
        m_pBody->Set_Visible(bVisible);

    // 안보일 때 히트박스 Off
    Enable_Colliders(bVisible);
    Enable_Controller(bVisible);
}

CMonsterBrain* CKabu::Create_Brain()
{
    return CKabu_Brain::Create(this);
}

HRESULT CKabu::Create_Movement()
{
    m_pMovement = Add_Component<CMonster_RailMovement>(TEXT("Com_Movement"), CMonster_RailMovement::Create(m_pDevice, m_pContext));

    if (nullptr == m_pMovement)
        return E_FAIL;

    static_cast<CMonster_RailMovement*>(m_pMovement)->Set_SpinSpeed(720.f);

    return S_OK;
}

HRESULT CKabu::Ready_State()
{
    if (m_pStateMachine == nullptr)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    // IDLE - 이후 CKabu_State_Idle 로 교체
    Info.strAniName = "Wait";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CKabu_State_Idle::Create(Info))))
        return E_FAIL;

    // Fall
    Info.strAniName = "Fall";
    Info.bLoop = true;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    // Landing
    Info.strAniName = "Landing";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Damage";
    Info.bLoop = false;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
        return E_FAIL;

    // Captured / Spat → Damage(loop)
    Info.bLoop = true;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
        return E_FAIL;

    // Warp_Out 
    Info.strAniName = "Warp1";
    Info.bLoop = false;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::WARPOUT, CKabu_State_WarpOut::Create(Info))))
        return E_FAIL;

    Info.strAniName = "Warp2";
    Info.bLoop = false;
    Info.fSpeed = 1.5f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::WARPIN, CKabu_State_WarpIn::Create(Info))))
        return E_FAIL;

    return S_OK;
}

HRESULT CKabu::Ready_AnimEvents()
{
    if (nullptr == m_pBody)
        return E_FAIL;

    CAnimator* pAnimator = m_pBody->Get_Animator();
    if (nullptr == pAnimator)
        return E_FAIL;

    pAnimator->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (Handle_SharedAnimEvent(e, ePhase))
                return;

            //switch (static_cast<EANIM_EVENT>(e.iEventType))
            //{
            //default:
            //    break;
            //}
        });

    return S_OK;
}

HRESULT  CKabu::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CKabu_Body>(
        CKabu_Body::PROTOTYPE_TAG, TEXT("Body"));

    if (nullptr == m_pBody)
        return E_FAIL;

    return S_OK;
}

void CKabu::On_Deserialized()
{
    __super::On_Deserialized();
}

CKabu* CKabu::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKabu* pInstance = new CKabu(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKabu");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CKabu::Clone(void* pArg)
{
    CKabu* pInstance = new CKabu(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CKabu");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CKabu::Free()
{
    __super::Free();
}
