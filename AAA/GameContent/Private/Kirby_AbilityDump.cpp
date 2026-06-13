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
    return S_OK;
}

KIRBY_STATE_TYPE CKirby_AbilityDump::Get_StateType()
{
    return KIRBY_STATE_TYPE::ABILITY_DUMP;
}

void CKirby_AbilityDump::Enter(CKirby* pKirby)
{
    // Ani
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::ABILITY_DUMP));

    m_bPartsOff = false;
    m_bCloseEye = false;
}

void CKirby_AbilityDump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    const _float fRatio = pAnimator->Get_Progress();

    Parts_Off(pKirby, fRatio);
    Close_Eye(pBody, fRatio);
}

void CKirby_AbilityDump::Exit(CKirby* pKirby)
{
}

_bool CKirby_AbilityDump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

void CKirby_AbilityDump::Parts_Off(CKirby* pKirby, _float fRatio)
{
    if (m_bPartsOff == false && fRatio >= 0.5f)
    {
        pKirby->OnOffParts(pKirby->Get_KirbyAbility()->Get_AbilityType(), false);
        pKirby->Set_KirbyAbility(KIRBY_ABILITY_TYPE::NORMAL);
        m_bPartsOff = true;
    }
}

void CKirby_AbilityDump::Close_Eye(CKirby_Body* pBody, _float fRatio)
{
    if (m_bCloseEye == false && fRatio >= 0.5f && fRatio < 0.8f)
    {
        pBody->Set_Eye(KIRBY_EYE_STATE::CLOSE);

        m_bCloseEye = true;
    }
    else if (m_bCloseEye == true && fRatio >= 0.8f)
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
