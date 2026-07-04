#include "Kirby_Ladder.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

CKirby_Ladder::CKirby_Ladder()
{
}

HRESULT CKirby_Ladder::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Ladder::Get_StateType()
{
    return KIRBY_STATE_TYPE::LADDER;
}

void CKirby_Ladder::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);
    
    pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);
    pKirby->Get_Body()->Get_Animator()->Play("LadderWait", true, false, 0.1f, 1.5f);
}

void CKirby_Ladder::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

}

void CKirby_Ladder::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_Ladder::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    //switch (eCommandType)
    //{
    //    // Move Press
    //    case KIRBY_COMMAND_TYPE::MOVE_TOP:
    //    case KIRBY_COMMAND_TYPE::MOVE_DOWN:
    //    case KIRBY_COMMAND_TYPE::MOVE_LEFT:
    //    case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
    //    {
    //        if (!pCommand->IsPress())
    //            return false;

    //        Handle_MoveCommand(pKirby, pCommand);
    //        pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
    //        return true;
    //    }
    //    // Jump Down
    //    case KIRBY_COMMAND_TYPE::JUMP:
    //    {
    //        if (!pCommand->IsDown())
    //            return false;

    //        pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
    //        return true;
    //    }
    //}

    return false;
}

CKirby_Ladder* CKirby_Ladder::Create()
{
    CKirby_Ladder* pInstance = new CKirby_Ladder();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ladder");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ladder::Free()
{
    __super::Free();
}
