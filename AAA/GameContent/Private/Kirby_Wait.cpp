#include "Kirby_Wait.h"

#include "GameInstance.h"

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
}

void CKirby_Wait::Update(CKirby* pKirby, const _float fTimeDelta)
{
}

void CKirby_Wait::End(CKirby* pKirby)
{
}

_bool CKirby_Wait::Handle_Command(CKirby* pKirby, const CKirby_Command& Command)
{
    __super::Handle_Command(pKirby, Command);



    return true;
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
