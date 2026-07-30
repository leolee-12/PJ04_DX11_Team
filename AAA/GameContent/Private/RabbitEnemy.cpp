#include "RabbitEnemy.h"
#include "GameInstance.h"
#include "GameContent_AnimEvents.h"
#include "Monster_StateMachine.h"

#include "RabbitEnemy_Body.h"
#include "RabbitEnemy_Brain.h"
#include "Monster_Movement.h"

#include "RabbitEnemy_State_Jump.h"
#include "Monster_State_Idle.h"
#include "Monster_State_Detect.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"


CRabbitEnemy::CRabbitEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CRabbitEnemy::CRabbitEnemy(const CRabbitEnemy& Prototype)
    : CMonster(Prototype)
{
}

HRESULT CRabbitEnemy::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CRabbitEnemy::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
    m_fCullDist = 175.f;

    m_TraitFlags |= MT_STRONG_INHALE_ONLY;

    if (m_pMovement)
        m_pMovement->Set_Stats(6.f, 720.f, -20.f, 14.f);

    return S_OK;
}

_bool CRabbitEnemy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { 1.25f };
    Out.fHeight = { 0.1f };
    return true;
}

CAnimator* CRabbitEnemy::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CRabbitEnemy::Create_Brain()
{
    return CRabbitEnemy_Brain::Create(this);
}

HRESULT CRabbitEnemy::Ready_State()
{
    if (nullptr == m_pStateMachine)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    // IDLE (Wait) - stationary
    Info.strAniName = "Wait";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
        return E_FAIL;

    // DETECT (Find) -> JUMP
    Info.strAniName = "Find";
    Info.bLoop = false;
    Info.fSpeed = 1.25f;

    CMonster_State_Detect* pDetect = CMonster_State_Detect::Create(Info);
    if (nullptr == pDetect)
        return E_FAIL;
    pDetect->Set_NextState(MONSTER_STATE_TYPE::JUMP);

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::DETECT, pDetect)))
        return E_FAIL;

    // JUMP (in-place hop impulse)
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::JUMP, CRabbitEnemy_State_Jump::Create())))
        return E_FAIL;

    // FALL (JumpFall) -> LANDING
    Info.strAniName = "JumpFall";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    // LANDING (JumpLanding) -> IDLE
    Info.strAniName = "JumpLanding";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::LANDING, CMonster_State_Landing::Create(Info))))
        return E_FAIL;

    // Damage (KnockBack / KnockBackDeath / KnockOut)
    Info.strAniName = "Damage";
    Info.bLoop = false;
    Info.fSpeed = 1.25f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK, CMonster_State_KnockBack::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_BACK_DEATH, CMonster_State_KnockBackDeath::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::KNOCK_OUT, CMonster_State_KnockOut::Create(Info))))
        return E_FAIL;

    // Captured / Spat (Damage loop)
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CAPTURED, CMonster_State_Captured::Create(Info))))
        return E_FAIL;

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SPAT, CMonster_State_Spat::Create(Info))))
        return E_FAIL;

    return S_OK;
}

void CRabbitEnemy::Apply_AIVariation(const _wstring& strVariation)
{
    // Only "Wait" observed. All map to stationary-react (0).
    if (strVariation == L"Wait")
        m_iAIType = 0;
    else
        m_iAIType = 0;
}

HRESULT CRabbitEnemy::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CRabbitEnemy_Body>(CRabbitEnemy_Body::PROTOTYPE_TAG, TEXT("Body"));
    if (nullptr == m_pBody)
        return E_FAIL;

    return S_OK;
}

HRESULT CRabbitEnemy::Ready_AnimEvents()
{
    if (nullptr == m_pBody)
        return E_FAIL;

    CAnimator* pAnim = m_pBody->Get_Animator();
    if (nullptr == pAnim)
        return E_FAIL;

    pAnim->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
        {
            if (Handle_SoundAnimEvent(e, phase))
                return;

            switch (static_cast<EANIM_EVENT>(e.iEventType))
            {
            case EANIM_EVENT::SetEye:
                if (phase == ANIM_EVENT_PHASE::POINT)
                    m_pBody->Set_Eye(static_cast<_uint>(e.iIntParam));
                break;

            default:
                break;
            }
        });

    return S_OK;
}

void CRabbitEnemy::On_Exit(MONSTER_STATE_TYPE eNextState)
{
    if (nullptr == m_pBody)
        return;

    switch (eNextState)
    {
    case MONSTER_STATE_TYPE::KNOCK_BACK:
    case MONSTER_STATE_TYPE::KNOCK_BACK_DEATH:
    case MONSTER_STATE_TYPE::KNOCK_OUT:
    case MONSTER_STATE_TYPE::CAPTURED:
    case MONSTER_STATE_TYPE::SPAT:
        break;

    default:
        m_pBody->Set_Eye(0);
        break;
    }
}

CRabbitEnemy* CRabbitEnemy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRabbitEnemy* pInstance = new CRabbitEnemy(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CRabbitEnemy");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRabbitEnemy::Clone(void* pArg)
{
    CRabbitEnemy* pInstance = new CRabbitEnemy(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CRabbitEnemy");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRabbitEnemy::Free()
{
    __super::Free();
}
