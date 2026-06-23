#include "Kirby_Damaged.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Damaged::CKirby_Damaged()
{
}

HRESULT CKirby_Damaged::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Damaged::Get_StateType()
{
    return KIRBY_STATE_TYPE::DAMAGED;
}

void CKirby_Damaged::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::DAMAGED);
}

void CKirby_Damaged::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    if (pAnimator->Get_Progress() >= 0.25f)
        Transition_Fall_OR_Wait_OR_Run(pKirby);
}

void CKirby_Damaged::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_Damaged::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

CKirby_Damaged* CKirby_Damaged::Create()
{
    CKirby_Damaged* pInstance = new CKirby_Damaged();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Damaged");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Damaged::Free()
{
    __super::Free();
}
