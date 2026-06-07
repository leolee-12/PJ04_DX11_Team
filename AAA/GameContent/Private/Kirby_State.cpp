#include "Kirby_State.h"

#include "GameInstance.h"


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

void CKirby_State::End(CKirby* pKirby)
{
}

_bool CKirby_State::Handle_Command(CKirby* pKirby, const CKirby_Command& Command)
{
    // 전역 처리

    return true;
}

void CKirby_State::Free()
{
    __super::Free();
}
