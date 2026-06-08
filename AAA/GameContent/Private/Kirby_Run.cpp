#include "Kirby_Run.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_Run::CKirby_Run()
{
}

HRESULT CKirby_Run::Initialize()
{

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Run::Get_StateType()
{
    return KIRBY_STATE_TYPE::RUN;
}

void CKirby_Run::Enter(CKirby* pKirby)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play("Run", true);

}

void CKirby_Run::Update(CKirby* pKirby, const _float fTimeDelta)
{
    if (pKirby->Has_MoveDir() == false)
        pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
}

void CKirby_Run::Exit(CKirby* pKirby)
{
}

_bool CKirby_Run::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
            Handle_MoveCommand(pKirby, pCommand);
            return true;

        case KIRBY_COMMAND_TYPE::JUMP:
            pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
            return true;
    }

    return false;
}

CKirby_Run* CKirby_Run::Create()
{
    CKirby_Run* pInstance = new CKirby_Run();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Run");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Run::Free()
{
    __super::Free();
}