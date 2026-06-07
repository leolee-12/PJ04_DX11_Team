#include "Kirby_InputManager.h"

#include "GameInstance.h"

#include "Kirby_Controller.h"
#include "Kirby_Command.h"

CKirby_InputManager::CKirby_InputManager()
{
}

HRESULT CKirby_InputManager::Initialize(CKirby_Controller* pKirby_Controller)
{
    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    m_pKirby_Controller = pKirby_Controller;
    if (m_pKirby_Controller == nullptr)
        return E_FAIL;

	return S_OK;
}

void CKirby_InputManager::Update_KirbyInput(_float fTimeDelta)
{
    CKirby_Command* pCommand{};

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W))
    {
        pCommand = new MoveTop_Command;
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A))
    {
        pCommand = new MoveBottom_Command;
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S))
    {
        pCommand = new MoveLeft_Command;
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D))
    {
        pCommand = new MoveRight_Command;
    }

    if (m_pGameInstance_Proxy->Key_Down(DIK_Z))
    {
        pCommand = new Jump_Command;
    }

    if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON))
    {
        pCommand = new ATTACK_Command;
    }

    m_pKirby_Controller->Push_Command(pCommand);
}

CKirby_InputManager* CKirby_InputManager::Create(CKirby_Controller* pKirby_Controller)
{
    CKirby_InputManager* pInstance = new CKirby_InputManager();

    if (FAILED(pInstance->Initialize(pKirby_Controller)))
    {
        MSG_BOX("Failed to Created: CKirby_InputManager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_InputManager::Free()
{
    Safe_Release(m_pGameInstance_Proxy);

	__super::Free();
}
