#include "Kirby_InputManager.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Controller.h"
#include "Kirby_Command.h"

CKirby_InputManager::CKirby_InputManager()
{
}

HRESULT CKirby_InputManager::Initialize(CKirby* pKiryby, CKirby_Controller* pKirby_Controller)
{
    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    m_pKirby_Controller = pKirby_Controller;
    if (m_pKirby_Controller == nullptr)
        return E_FAIL;

    m_pKiryby = pKiryby;
    if (m_pKiryby == nullptr)
        return E_FAIL;

	return S_OK;
}

void CKirby_InputManager::Update_KirbyInput(_float fTimeDelta)
{
    CKirby_Command* pCommand{};

    // Top
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::TOP, vDir) == true)
        {
            pCommand = new MoveTop_Command(KEY_STATE_TYPE::PRESS, vDir);
            ProcessCommand(pCommand);
        }
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_W))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::TOP, vDir) == true)
        {
            pCommand = new MoveTop_Command(KEY_STATE_TYPE::DOWN, vDir);
            ProcessCommand(pCommand);
        }
    }

    // DOWN
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::DOWN, vDir))
        {
            pCommand = new MoveBottom_Command(KEY_STATE_TYPE::PRESS, vDir);
            ProcessCommand(pCommand);
        }
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_S))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::DOWN, vDir))
        {
            pCommand = new MoveBottom_Command(KEY_STATE_TYPE::DOWN, vDir);
            ProcessCommand(pCommand);
        }
    }

    // LEFT
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::LEFT, vDir))
        {
            pCommand = new MoveLeft_Command(KEY_STATE_TYPE::PRESS, vDir);
            ProcessCommand(pCommand);
        }
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_A))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::LEFT, vDir))
        {
            pCommand = new MoveLeft_Command(KEY_STATE_TYPE::DOWN, vDir);
            ProcessCommand(pCommand);
        }
    }

    // RIGHT
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::RIGHT, vDir))
        {
            pCommand = new MoveRight_Command(KEY_STATE_TYPE::PRESS, vDir);
            ProcessCommand(pCommand);
        }
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_D))
    {
        _float3 vDir{};
        if (Cal_MoveDir(MOVE_DIR::RIGHT, vDir))
        {
            pCommand = new MoveRight_Command(KEY_STATE_TYPE::DOWN, vDir);
            ProcessCommand(pCommand);
        }
    }

    // Jump
    // A
    if (m_pGameInstance_Proxy->Key_Down(DIK_SPACE))
    {
        pCommand = new Jump_Command(KEY_STATE_TYPE::DOWN);
        ProcessCommand(pCommand);
    }
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_SPACE))
    {
        pCommand = new Jump_Command(KEY_STATE_TYPE::PRESS);
        ProcessCommand(pCommand);
    }

    // B
    if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON))
    {
        pCommand = new ATTACK_Command(KEY_STATE_TYPE::DOWN);
        ProcessCommand(pCommand);
    }
    if (m_pGameInstance_Proxy->Mouse_Pressing(DIMB::LBUTTON))
    {
        pCommand = new ATTACK_Command(KEY_STATE_TYPE::PRESS);
        ProcessCommand(pCommand);
    }
    if (m_pGameInstance_Proxy->Mouse_Up(DIMB::LBUTTON))
    {
        pCommand = new ATTACK_Command(KEY_STATE_TYPE::UP);
        ProcessCommand(pCommand);
    }

    // Y
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_X))
    {
        pCommand = new Dump_Command(KEY_STATE_TYPE::PRESS);
        ProcessCommand(pCommand);
    }

    // Q, E
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_Q) ||
        m_pGameInstance_Proxy->Key_Pressing(DIK_E))
    {
        pCommand = new Guard_Command(KEY_STATE_TYPE::PRESS);
        ProcessCommand(pCommand);
    }
    if (m_pGameInstance_Proxy->Key_Up(DIK_Q) ||
        m_pGameInstance_Proxy->Key_Up(DIK_E))
    {
        pCommand = new Guard_Command(KEY_STATE_TYPE::UP);
        ProcessCommand(pCommand);
    }
}

_bool CKirby_InputManager::Cal_MoveDir(MOVE_DIR eMoveDir, _float3& vOutDir)
{
    CMovement_Child* pMovement = m_pKiryby->Get_Component<CMovement_Child>(TEXT("Com_Movement"));
    if (pMovement == nullptr)
        return false;

    _vector vWishDir = XMVectorZero();
    _vector vCamLook = XMLoadFloat4(m_pGameInstance_Proxy->Get_CamLook());

    _vector vCamFwd = XMVector3Normalize(XMVectorSetY(vCamLook, 0.f));
    _vector vCamRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vCamFwd));

    switch (eMoveDir)
    {
        case MOVE_DIR::TOP:
            vWishDir += vCamFwd;
            break;
        case MOVE_DIR::DOWN:
            vWishDir -= vCamFwd;
            break;
        case MOVE_DIR::LEFT:
            vWishDir -= vCamRight;
            break;
        case MOVE_DIR::RIGHT:
            vWishDir += vCamRight;
            break;
    }

    XMStoreFloat3(&vOutDir, vWishDir);

    return true;
}

void CKirby_InputManager::ProcessCommand(CKirby_Command* pCommand)
{
    if (pCommand != nullptr)
        m_pKirby_Controller->Push_Command(pCommand);
    pCommand = nullptr;
}

CKirby_InputManager* CKirby_InputManager::Create(CKirby* pKiryby, CKirby_Controller* pKirby_Controller)
{
    CKirby_InputManager* pInstance = new CKirby_InputManager();

    if (FAILED(pInstance->Initialize(pKiryby, pKirby_Controller)))
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
