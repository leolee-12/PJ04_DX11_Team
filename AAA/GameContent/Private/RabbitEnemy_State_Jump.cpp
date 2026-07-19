#include "RabbitEnemy_State_Jump.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CRabbitEnemy_State_Jump::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    // let the hop arc complete without brain interference
    m_bIsInterruptible = false;
    return S_OK;
}

MONSTER_STATE_TYPE CRabbitEnemy_State_Jump::Get_StateType()
{
    return MONSTER_STATE_TYPE::JUMP;
}

void CRabbitEnemy_State_Jump::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (nullptr == m_pAnimator || nullptr == m_pMovement)
        return;

    m_bJumped = false;
    Begin_Jump();
}

void CRabbitEnemy_State_Jump::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner || nullptr == m_pAnimator || nullptr == m_pMovement)
        return;


    if (!m_bJumped && m_pAnimator->Get_CurrentAnimName() == "Jump")
    {
        m_pMovement->Jump();
        m_bJumped = true;
    }

    if (m_bJumped && m_pMovement->Is_Grounded() && m_pAnimator->Is_Finished())
        m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CRabbitEnemy_State_Jump::Exit(MONSTER_STATE_TYPE eNextState)
{
}

void CRabbitEnemy_State_Jump::Begin_Jump()
{
    if (nullptr == m_pAnimator)
        return;

    m_bJumped = false;

    ANI_PLAY_INFO Info{};
    Info.strAniName = "JumpStart";
    Info.bLoop = false;
    Info.fSpeed = 1.f;
    m_pAnimator->Play(&Info);

    Info.strAniName = "Jump";
    Info.bLoop = false;
    Info.fSpeed = 1.f;
    m_pAnimator->Enqueue(Info);
}

CRabbitEnemy_State_Jump* CRabbitEnemy_State_Jump::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CRabbitEnemy_State_Jump* pInstance = new CRabbitEnemy_State_Jump();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CRabbitEnemy_State_Jump");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRabbitEnemy_State_Jump::Free()
{
    __super::Free();
}
