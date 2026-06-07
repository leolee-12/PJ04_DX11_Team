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
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W))
    {
        m_pKirby_Controller->Push_Command(MoveTop_Command());
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A))
    {
        m_pKirby_Controller->Push_Command(MoveBottom_Command());
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S))
    {
        m_pKirby_Controller->Push_Command(MoveLeft_Command());
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D))
    {
        m_pKirby_Controller->Push_Command(MoveRight_Command());
    }

    if (m_pGameInstance_Proxy->Key_Down(DIK_Z))
    {
        m_pKirby_Controller->Push_Command(Jump_Command());
    }

    if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON))
    {
        m_pKirby_Controller->Push_Command(ATTACK_Command());
    }
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
