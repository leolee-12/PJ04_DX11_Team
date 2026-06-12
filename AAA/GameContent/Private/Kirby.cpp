#include "Kirby.h"

#include "GameInstance.h"

#include "PartObject.h"

#include "GameContent_const.h"
#include "Movement_Child.h"

#include "Kirby_Body.h"

#include "Kirby_InputManager.h"
#include "Kirby_Controller.h"
#include "Kirby_StateMachine.h"

// Ability
#include "Kirby_Ability_Normal.h"

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

    if (FAILED(Ready_Ability()))
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

    if(Has_MoveDir())
    {
        _vector vDir = XMLoadFloat3(&m_vWishDir);
        m_pMovement->Add_Acceleration(vDir * 120.f);
        m_pMovement->Rotate_To_Direction(vDir, fTimeDelta);
    }

    m_pMovement->Update_RigidBody(fTimeDelta);
}

void CKirby::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_pTriggerSensor && m_pTransformCom)
    {
        m_pTriggerSensor->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_pTriggerSensor);
#endif
    }
}

HRESULT CKirby::Render()
{
    return S_OK;
}

void CKirby::Add_MoveDir(const _float3& vWishDir)
{
    XMStoreFloat3(&m_vWishDir,
        XMLoadFloat3(&vWishDir) + XMLoadFloat3(&m_vWishDir));
}

_bool CKirby::Has_MoveDir()
{
    _vector vWishDir = XMLoadFloat3(&m_vWishDir);

    if (XMVector3Equal(vWishDir, XMVectorZero()))
        return false;

    return true;
}

void CKirby::Excute_Command(CKirby_Command* pCommand)
{
    m_pKirby_StateMachine->Handle_Command(pCommand);
}

void CKirby::Change_State(KIRBY_STATE_TYPE eNewState)
{
    m_pKirby_StateMachine->Change_State(eNewState);
}

CKirby_Ability* CKirby::Get_KirbyAbility()
{
    return m_pKirby_Ability;
}

void CKirby::Set_KirbyAbility(CKirby_Ability* pKirby_Ability)
{
    if (m_pKirby_Ability != nullptr)
        Safe_Release(m_pKirby_Ability);

    m_pKirby_Ability = pKirby_Ability;
}

HRESULT CKirby::Ready_Components()
{
    _float3 vFootPos;
    XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));
    m_pController = m_pGameInstance_Proxy->Create_CapsuleController(vFootPos, CCT_RADIUS, CCT_HEIGHT);

    m_pMovement = Add_Component<CMovement_Child>(TEXT("Com_Movement"), CMovement_Child::Create(m_pDevice, m_pContext));
    if (m_pMovement == nullptr)
        return E_FAIL;

    m_pMovement->Set_Refs(m_pTransformCom, m_pController);

    // TriggerSensor(Collider)
    m_pTriggerSensor = Add_Component<CCollider>(
        TEXT("Com_TriggerSensor"),
        CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));
    if (nullptr == m_pTriggerSensor)
        return E_FAIL;

    CCollider::COLLIDER_DESC ColliderDesc{};
    ColliderDesc.pOwner = this;
    ColliderDesc.vCenter = _float3(0.f, CCT_RADIUS + CCT_HEIGHT * 0.5f, 0.f);
    ColliderDesc.vSize = _float3(
        CCT_RADIUS * 2.f,
        CCT_HEIGHT + CCT_RADIUS * 2.f,
        CCT_RADIUS * 2.f);

    if (FAILED(m_pTriggerSensor->Initialize(&ColliderDesc)))
        return E_FAIL;

    m_pGameInstance_Proxy->Register_Collider(m_pTriggerSensor, CL_PLAYER_SENSOR);
    m_pGameInstance_Proxy->Add_CollisionPool(CL_PLAYER_SENSOR, CL_ENV_TRIGGER);

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

HRESULT CKirby::Ready_Ability()
{
    m_pKirby_Ability = CKirby_Ability_Normal::Create();
    if (m_pKirby_Ability == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby::Bind_ShaderResources()
{
    return S_OK;
}

void CKirby::On_Deserialized()
{
    if (m_pMovement)
        m_pMovement->Sync_To_Controller();
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
    Safe_Release(m_pKirby_Ability);

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