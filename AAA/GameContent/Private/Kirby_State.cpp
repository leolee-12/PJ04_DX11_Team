#include "Kirby_State.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_State::CKirby_State()
{
}

HRESULT CKirby_State::Initialize()
{

    return S_OK;
}

void CKirby_State::Enter(CKirby* pKirby)
{
}

void CKirby_State::Update(CKirby* pKirby, const _float fTimeDelta)
{
}

void CKirby_State::Exit(CKirby* pKirby)
{
}

_bool CKirby_State::Handle_MoveCommand(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    if (eCommandType == KIRBY_COMMAND_TYPE::MOVE_TOP || eCommandType == KIRBY_COMMAND_TYPE::MOVE_DOWN ||
        eCommandType == KIRBY_COMMAND_TYPE::MOVE_LEFT || eCommandType == KIRBY_COMMAND_TYPE::MOVE_RIGHT)
    {
        Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
        pKirby->Add_MoveDir(pMoveCommand->Get_Dir());

        return true;
    }

    return false;
}

_bool CKirby_State::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    // 전역 처리

    return false;
}

void CKirby_State::Free()
{
    __super::Free();
}
