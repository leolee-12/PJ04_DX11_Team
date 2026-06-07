#include "Kirby.h"

#include "GameInstance.h"

#include "PartObject.h"

#include "GameContent_const.h"
#include "Kirby_Body.h"

#include "Kirby_InputManager.h"
#include "Kirby_Controller.h"
#include "Kirby_StateMachine.h"

CKirby::CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CKirby::CKirby(const CKirby& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CKirby::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (FAILED(Ready_System()))
        return E_FAIL;
  
    return S_OK;
}

void CKirby::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CKirby::Update(_float fTimeDelta)
{
    XMStoreFloat3(&m_vWishDir, XMVectorZero());

    __super::Update(fTimeDelta);

    m_pKirby_InputManager->Update_KirbyInput(fTimeDelta);
    m_pKirby_Controller->Update_KirbyController(fTimeDelta);
    m_pKirby_StateMachine->Update_StateMachine(fTimeDelta);


    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        m_pMovement->Sync_To_Controller();
        return;
    }

    //if (m_pGameInstance_Proxy->Key_Down(DIK_SPACE))
    //    m_pMovement->Jump();

    m_pMovement->Move(XMVectorSetW(XMLoadFloat3(&m_vWishDir), 0), fTimeDelta);
}

void CKirby::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CKirby::Render()
{
    return S_OK;
}

void CKirby::Add_WishDir(const _float3& vWishDir)
{
    XMStoreFloat3(&m_vWishDir,
        XMLoadFloat3(&vWishDir) + XMLoadFloat3(&m_vWishDir));
}

void CKirby::Excute_Command(CKirby_Command* pCommand)
{
    m_pKirby_StateMachine->Handle_Command(pCommand);
}

void CKirby::Change_State(KIRBY_STATE_TYPE eNewState)
{
    m_pKirby_StateMachine->Change_State(eNewState);
}

HRESULT CKirby::Ready_Components()
{
    _float3 vFootPos;
    XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));
    m_pController = m_pGameInstance_Proxy->Create_CapsuleController(vFootPos, CCT_RADIUS, CCT_HEIGHT);

    m_pMovement = Add_Component<CMovement>(TEXT("Com_Movement"), CMovement::Create(m_pDevice, m_pContext));
    if (m_pMovement == nullptr)
        return E_FAIL;

    m_pMovement->Set_Refs(m_pTransformCom, m_pController);
    m_pMovement->Set_Stats(MOVE_SPEED, ROT_SPEED, GRAVITY, JUMP_SPEED);
    m_pMovement->Set_Acceleration(MOVE_ACCEL, MOVE_DECEL);

    return S_OK;
}

HRESULT CKirby::Ready_PartObjects()
{
    CKirby_Body::KIRBY_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(ETOUI(LEVEL::GAMEPLAY), CKirby_Body::PROTOTYPE_TAG,
        TEXT("Body"), &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CKirby_Body*>(m_PartObjects[TEXT("Body")]);

    return S_OK;
}

HRESULT CKirby::Ready_System()
{
    m_pKirby_StateMachine = CKirby_StateMachine::Create(this);
    if (m_pKirby_StateMachine == nullptr)
        return E_FAIL;

    m_pKirby_Controller = CKirby_Controller::Create(this);
    if (m_pKirby_Controller == nullptr)
        return E_FAIL;

    m_pKirby_InputManager =  CKirby_InputManager::Create(this, m_pKirby_Controller);
    if (m_pKirby_InputManager == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby::Bind_ShaderResources()
{
    return S_OK;
}

CKirby* CKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby* pInstance = new CKirby(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby::Clone(void* pArg)
{
    CKirby* pInstance = new CKirby(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CKirby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby::Free()
{
    Safe_Release(m_pKirby_InputManager);
    Safe_Release(m_pKirby_Controller);
    Safe_Release(m_pKirby_StateMachine);

    if (m_pController != nullptr)
    {
        m_pGameInstance_Proxy->Release_Controller(m_pController);
        m_pController = nullptr;
    }

    __super::Free();
}