#include "Noddy.h"
#include "GameInstance.h"
#include "GameContent_AnimEvents.h"
#include "Monster_StateMachine.h"

#include "Noddy_Body.h"
#include "Noddy_Brain.h"
#include "Monster_Movement.h"

#include "Noddy_State_Chase.h"
#include "Noddy_State_SleepStart.h"
#include "Monster_State_Idle.h"
#include "Noddy_State_Find.h"
#include "Monster_State_Detect.h"
#include "Monster_State_Fall.h"
#include "Monster_State_Landing.h"
#include "Monster_State_KnockBack.h"
#include "Monster_State_KnockBackDeath.h"
#include "Monster_State_KnockOut.h"
#include "Monster_State_Captured.h"
#include "Monster_State_Spat.h"


CNoddy::CNoddy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonster{ pDevice, pContext }
{
}

CNoddy::CNoddy(const CNoddy& Prototype)
    : CMonster(Prototype)
{
}

HRESULT CNoddy::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CNoddy::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_eCopyAbility = COPY_ABILITY_TYPE::SLEEP;
    m_fCullDist = 95.f;

    if (m_pMovement)
        m_pMovement->Set_Stats(2.f, 720.f, -20.f, 0.f);

    return S_OK;
}

_bool CNoddy::Get_HurtBoxDesc(CAPSULE_DESC& Out) const
{
    Out.fRadius = { 0.65f };
    Out.fHeight = { 0.01f };
    return true;
}

CAnimator* CNoddy::Get_BodyAnimator() const
{
    return m_pBody ? m_pBody->Get_Animator() : nullptr;
}

CMonsterBrain* CNoddy::Create_Brain()
{
    return CNoddy_Brain::Create(this);
}

HRESULT CNoddy::Ready_State()
{
    if (nullptr == m_pStateMachine)
        return E_FAIL;

    if (FAILED(__super::Ready_State()))
        return E_FAIL;

    ANI_PLAY_INFO Info{};

    // IDLE (Sleep)
    Info.strAniName = "Sleep";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
        return E_FAIL;

    // DETECT (Wakeup) -> CHASE
    Info.strAniName = "Wakeup";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;

    CMonster_State_Detect* pDetect = CMonster_State_Detect::Create(Info);
    if (nullptr == pDetect)
        return E_FAIL;
    pDetect->Set_NextState(MONSTER_STATE_TYPE::CHASE);

    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::DETECT, pDetect)))
        return E_FAIL;

    // CHASE (Move) -> SLEEP_START when the target is lost
    Info.strAniName = "Move";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::CHASE, CNoddy_State_Chase::Create(Info, 2.0f))))
        return E_FAIL;

    // SLEEP_START (SleepStart) -> IDLE
    Info.strAniName = "SleepStart";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::SLEEP_START, CNoddy_State_SleepStart::Create(Info))))
        return E_FAIL;

    // Find
    Info.strAniName = "LookAround";
    Info.bLoop = false;
    Info.fSpeed = 1.25f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FIND,  CNoddy_State_Find::Create(Info))))
        return E_FAIL;

    // FALL -> LANDING
    Info.strAniName = "Fall";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::FALL, CMonster_State_Fall::Create(Info))))
        return E_FAIL;

    // LANDING -> IDLE
    Info.strAniName = "Landing";
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

void CNoddy::Apply_AIVariation(const _wstring& strVariation)
{
    if (strVariation == L"Pursuit")
        m_iAIType = 0;
    else
        m_iAIType = 0;
}

HRESULT CNoddy::Ready_PartObjects()
{
    m_pBody = Add_MonsterPart<CNoddy_Body>(CNoddy_Body::PROTOTYPE_TAG, TEXT("Body"));
    if (nullptr == m_pBody)
        return E_FAIL;

    return S_OK;
}

HRESULT CNoddy::Ready_AnimEvents()
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
            case EANIM_EVENT::SetBody:
                if (phase == ANIM_EVENT_PHASE::POINT)
                    m_pBody->Set_Face(static_cast<_uint>(e.iIntParam));
                break;

            default:
                break;
            }
        });

    return S_OK;
}

void CNoddy::On_Exit(MONSTER_STATE_TYPE eNextState)
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
        m_pBody->Set_Face(2);
        break;
    }
}

CNoddy* CNoddy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CNoddy* pInstance = new CNoddy(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CNoddy");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CNoddy::Clone(void* pArg)
{
    CNoddy* pInstance = new CNoddy(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CNoddy");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNoddy::Free()
{
    __super::Free();
}
