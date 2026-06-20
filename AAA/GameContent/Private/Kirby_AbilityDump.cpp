#include "Kirby_AbilityDump.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_AbilityDump::CKirby_AbilityDump()
{
}

HRESULT CKirby_AbilityDump::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_AbilityDump::Get_StateType()
{
    return KIRBY_STATE_TYPE::ABILITY_DUMP;
}

void CKirby_AbilityDump::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::ABILITY_DUMP));

    m_bPartsOff = false;
    m_bCloseEye = false;
}

void CKirby_AbilityDump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const _float fRatio = pAnimator->Get_Progress();


    Parts_Off(pKirby, fRatio);
    Close_Eye(pBody, fRatio);

    if(pAnimator->Is_Finished())
    {

        if (Try_FallState(pKirby) == true)
        {
            CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::FALL);
        }
        else
        {
            Transition_Wait_OR_Run(pKirby);
        }

        return;
    }
}

void CKirby_AbilityDump::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_AbilityDump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    //switch (eCommandType)
    //{
    //    default:
    //        break;
    //}

    return false;
}

void CKirby_AbilityDump::Parts_Off(CKirby* pKirby, _float fRatio)
{
    if (m_bPartsOff == false && fRatio >= 0.45f)
    {
        pKirby->OnOffParts(pKirby->Get_KirbyAbility()->Get_AbilityType(), false);
        pKirby->Request_ChangeKirbyAbility(COPY_ABILITY_TYPE::NORMAL);
        m_bPartsOff = true;
    }
}

void CKirby_AbilityDump::Close_Eye(CKirby_Body* pBody, _float fRatio)
{
    if (m_bCloseEye == false && fRatio >= 0.55f && fRatio < 0.9f)
    {
        pBody->Set_Eye(KIRBY_EYE_STATE::CLOSE);

        m_bCloseEye = true;
    }
    else if (m_bCloseEye == true && fRatio >= 0.9f)
    {
        pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);

        m_bCloseEye = false;
    }
}

CKirby_AbilityDump* CKirby_AbilityDump::Create()
{
    CKirby_AbilityDump* pInstance = new CKirby_AbilityDump();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_AbilityDump");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_AbilityDump::Free()
{
    __super::Free();
}
