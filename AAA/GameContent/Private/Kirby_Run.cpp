#include "Kirby_Run.h"

#include "GameInstance.h"

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
}

void CKirby_Run::Update(CKirby* pKirby, const _float fTimeDelta)
{
}

void CKirby_Run::End(CKirby* pKirby)
{
}

_bool CKirby_Run::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);



    return true;
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
