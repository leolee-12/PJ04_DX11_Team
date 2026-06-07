#include "Kirby_State.h"

#include "GameInstance.h"

#include "Kirby.h"


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

_bool CKirby_State::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    // 전역 처리
    if (pCommand->GetCommandType() == KIRBY_COMMAND_TYPE::MOVE_TOP ||
        pCommand->GetCommandType() == KIRBY_COMMAND_TYPE::MOVE_DOWN ||
        pCommand->GetCommandType() == KIRBY_COMMAND_TYPE::MOVE_LEFT ||
        pCommand->GetCommandType() == KIRBY_COMMAND_TYPE::MOVE_RIGHT)
    {
        Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
        _float3 vDir = pMoveCommand->Get_Dir();
        pKirby->Add_WishDir(vDir);
    }

    if (pCommand->GetCommandType() == KIRBY_COMMAND_TYPE::JUMP)
    {
        static_cast<CMovement*>(pKirby->Get_Component<CMovement>(TEXT("Com_Movement")))->Jump();
    }

    return true;
}

void CKirby_State::Free()
{
    __super::Free();
}
