#include "Kirby_Wait.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_Wait::CKirby_Wait()
{
}

HRESULT CKirby_Wait::Initialize()
{

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Wait::Get_StateType()
{
    return KIRBY_STATE_TYPE::WAIT;
}

void CKirby_Wait::Enter(CKirby* pKirby)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play("Wait", true, false, 0.1f, 1.8f);
}

void CKirby_Wait::Update(CKirby* pKirby, const _float fTimeDelta)
{
}

void CKirby_Wait::Exit(CKirby* pKirby)
{
}

_bool CKirby_Wait::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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
            pKirby->Change_State(KIRBY_STATE_TYPE::RUN);
            return true;

        case KIRBY_COMMAND_TYPE::JUMP:
            pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
            return true;
    }

    return false;
}

CKirby_Wait* CKirby_Wait::Create()
{
    CKirby_Wait* pInstance = new CKirby_Wait();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Wait");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Wait::Free()
{
    __super::Free();
}
