#include "Kirby_GetDeform.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_GetDeform::CKirby_GetDeform()
{
}

HRESULT CKirby_GetDeform::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_GetDeform::Get_StateType()
{
    return KIRBY_STATE_TYPE::GET_DEFORM;
}

void CKirby_GetDeform::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    pKirby->Set_KirbyDeform(DEFORM_TYPE::CAR);
}

void CKirby_GetDeform::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);
}

void CKirby_GetDeform::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_GetDeform::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

CKirby_GetDeform* CKirby_GetDeform::Create()
{
    CKirby_GetDeform* pInstance = new CKirby_GetDeform();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_GetDeform");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_GetDeform::Free()
{
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}