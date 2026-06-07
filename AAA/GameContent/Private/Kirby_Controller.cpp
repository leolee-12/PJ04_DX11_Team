#include "Kirby_Controller.h"

#include "GameInstance.h"

#include "Kirby.h"

CKirby_Controller::CKirby_Controller()
{
}

HRESULT CKirby_Controller::Initialize(CKirby* pPlayer)
{
    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    m_pPlayer = pPlayer;
    if(m_pPlayer == nullptr)
        return E_FAIL;

    return S_OK;
}

void CKirby_Controller::Update_KirbyController(_float fTimeDelta)
{
    while (m_CommandQueue.empty() == false)
    {
        CKirby_Command* pCommand = m_CommandQueue.front();
        m_pPlayer->Excute_Command(pCommand);

        m_CommandQueue.pop();

        Safe_Release(pCommand);
    }
}

void CKirby_Controller::Push_Command(CKirby_Command* pCommand)
{
    m_CommandQueue.push(pCommand);
}

CKirby_Controller* CKirby_Controller::Create(CKirby* pPlayer)
{
    CKirby_Controller* pInstance = new CKirby_Controller();

    if (FAILED(pInstance->Initialize(pPlayer)))
    {
        MSG_BOX("Failed to Created: CKirby_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Controller::Free()
{
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}
