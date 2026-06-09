#include "Kirby_Attack.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_Attack::CKirby_Attack()
{
}

HRESULT CKirby_Attack::Initialize()
{

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Attack::Get_StateType()
{
    return KIRBY_STATE_TYPE::ATTACK;
}

void CKirby_Attack::Enter(CKirby* pKirby)
{
}

void CKirby_Attack::Update(CKirby* pKirby, const _float fTimeDelta)
{
}

void CKirby_Attack::Exit(CKirby* pKirby)
{
}

_bool CKirby_Attack::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    //switch (eCommandType)
    //{
    //case KIRBY_COMMAND_TYPE::MOVE_TOP:
    //case KIRBY_COMMAND_TYPE::MOVE_DOWN:
    //case KIRBY_COMMAND_TYPE::MOVE_LEFT:
    //case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
    //    Handle_MoveCommand(pKirby, pCommand);
    //    pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
    //    return true;

    //case KIRBY_COMMAND_TYPE::JUMP:
    //    pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
    //    return true;
    //}

    return false;
}

CKirby_Attack* CKirby_Attack::Create()
{
    CKirby_Attack* pInstance = new CKirby_Attack();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Attack");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Attack::Free()
{
    __super::Free();
}
