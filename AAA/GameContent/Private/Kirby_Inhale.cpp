#include "Kirby_Inhale.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Inhale::CKirby_Inhale()
{
}

HRESULT CKirby_Inhale::Initialize()
{
    m_MaxSuperInHaleTime = 1.f;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Inhale::Get_StateType()
{
    return KIRBY_STATE_TYPE::INHALE;
}

void CKirby_Inhale::Enter(CKirby* pKirby)
{
    // Inhale State
    m_eInhaleState = INHALE_STATE::INHALE;

    // Super Inhale Timer
    m_AccSuperInHaleTime = 0.f;

    // Inhale Animation
    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();
    pAnimator->Play("Inhale", true, false, 0.1f, 1.5f);

    // Inhale Body
    pKirby_Body->Set_Body(KIRBY_BODY_STATE::INHALE);
}

void CKirby_Inhale::Update(CKirby* pKirby, const _float fTimeDelta)
{
    // Super Inhale Timer
    if(m_AccSuperInHaleTime < m_MaxSuperInHaleTime)
        m_AccSuperInHaleTime += fTimeDelta;

    CKirby_Body* pKirby_Body = pKirby->Get_Body();
    CAnimator* pAnimator = pKirby_Body->Get_Animator();

    // Inhale 종료
    if (m_eInhaleState != INHALE_STATE::INHALE_END && pKirby->Get_KirbyAbility()->IsFinished() == true)
    {
        m_eInhaleState = INHALE_STATE::INHALE_END;
        pAnimator->Play("InhaleEnd", false, false, 0.1f, 1.5f);
    }
    if (m_eInhaleState == INHALE_STATE::INHALE_END)
    {
        if (pAnimator->Get_Progress() >= 0.5f)
            pKirby_Body->Set_Body(KIRBY_BODY_STATE::NORMAL);

        if(pAnimator->Is_Finished() == true)
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
            pKirby_Body->Set_Eye(KIRBY_EYE_STATE::IDLE);
        }
        return;
    }

    // Inhale 강화
    if (m_eInhaleState == INHALE_STATE::INHALE &&
        m_AccSuperInHaleTime >= m_MaxSuperInHaleTime)
    {
        m_eInhaleState = INHALE_STATE::SUPER_INHALE_START;
        pAnimator->Play("SuperInhaleStart", false, false, 0.1f, 1.5f);
        pKirby_Body->Set_Eye(KIRBY_EYE_STATE::ANGRY);
    }
    else if (m_eInhaleState == INHALE_STATE::SUPER_INHALE_START &&
        pAnimator->Is_Finished() == true)
    {
        m_eInhaleState = INHALE_STATE::SUPER_INHALE_LOOP;
        pAnimator->Play("SuperInhale", true, false, 0.1f, 1.5f);
        pKirby_Body->Set_Eye(KIRBY_EYE_STATE::CLOSE);
    }
}

void CKirby_Inhale::Exit(CKirby* pKirby)
{
}

_bool CKirby_Inhale::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        case KIRBY_COMMAND_TYPE::ATTACK_UP:
            pKirby->Get_KirbyAbility()->Up_Attack(pKirby);
            return true;
    }

    return false;
}

CKirby_Inhale* CKirby_Inhale::Create()
{
    CKirby_Inhale* pInstance = new CKirby_Inhale();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Inhale");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Inhale::Free()
{
    __super::Free();
}
