#include "Kirby_GetAbility.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_GetAbility::CKirby_GetAbility()
{
}

HRESULT CKirby_GetAbility::Initialize()
{
    return S_OK;
}

KIRBY_STATE_TYPE CKirby_GetAbility::Get_StateType()
{
    return KIRBY_STATE_TYPE::GET_ABILITY;
}

void CKirby_GetAbility::Enter(CKirby* pKirby)
{
    // Ani
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::GET_ABILITY));

    m_bPartsOn = false;
    m_bCloseEye = false;
}

void CKirby_GetAbility::Update(CKirby* pKirby, const _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    const _float fRatio = pAnimator->Get_Progress();


    if(pAnimator->Is_Finished() == true)
    {
        // Fall
        if (Try_FallState(pKirby) == true)
        {
            pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::FALL));
        }
        // Wait or Run
        Transition_Wait_OR_Run(pKirby);
    }

    Parts_On(pKirby, fRatio);
    Close_Eye(pBody, fRatio);
}

void CKirby_GetAbility::Exit(CKirby* pKirby)
{
}

_bool CKirby_GetAbility::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        default:        
            break;
    }

    return false;
}

void CKirby_GetAbility::Parts_On(CKirby* pKirby, _float fRatio)
{
    if (m_bPartsOn == false && fRatio >= 0.3f)
    {
        pKirby->OnOffParts(pKirby->Get_KirbyAbility()->Get_AbilityType(), true);

        m_bPartsOn = true;
    }
}

void CKirby_GetAbility::Close_Eye(CKirby_Body* pBody, _float fRatio)
{
    if (m_bCloseEye == false && fRatio >= 0.3f && fRatio < 0.55f)
    {
        pBody->Set_Eye(KIRBY_EYE_STATE::CLOSE);

        m_bCloseEye = true;
    }
    else if (m_bCloseEye == true && fRatio >= 0.55f)
    {
        pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);

        m_bCloseEye = false;
    }
}

CKirby_GetAbility* CKirby_GetAbility::Create()
{
    CKirby_GetAbility* pInstance = new CKirby_GetAbility();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_GetAbility");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_GetAbility::Free()
{
    __super::Free();
}
