#include "Kirby.h"

#include "GameInstance.h"

#include "PartObject.h"

#include "GameContent_const.h"
#include "Kirby_Body.h"

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

   

    return S_OK;
}

void CKirby::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CKirby::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (nullptr == m_pMovement)
        return;

    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        m_pMovement->Sync_To_Controller();
        return;
    }

    _vector vWishDir = XMVectorZero();
    _vector vCamLook = XMLoadFloat4(m_pGameInstance_Proxy->Get_CamLook());

    _vector vCamFwd = XMVector3Normalize(XMVectorSetY(vCamLook, 0.f));                      
    _vector vCamRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vCamFwd)); 

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W)) 
        vWishDir += vCamFwd;

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S)) 
        vWishDir -= vCamFwd;

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D)) 
        vWishDir += vCamRight;

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A)) 
        vWishDir -= vCamRight;

    if (m_pGameInstance_Proxy->Key_Down(DIK_SPACE))
        m_pMovement->Jump();

    m_pMovement->Move(vWishDir, fTimeDelta);
}

void CKirby::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CKirby::Render()
{
    return S_OK;
}

void CKirby::Excute_Command(CKirby_Command* pCommand)
{

}

HRESULT CKirby::Ready_Components()
{
    _float3 vFoot;
    XMStoreFloat3(&vFoot, m_pTransformCom->Get_State(STATE::POSITION));
    m_pController = m_pGameInstance_Proxy->Create_CapsuleController(vFoot, CCT_RADIUS, CCT_HEIGHT);

    m_pMovement = Add_Component<CMovement>(TEXT("Com_Movement"),
        CMovement::Create(m_pDevice, m_pContext));
    if (nullptr == m_pMovement)
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
    if (m_pController)
    {
        m_pGameInstance_Proxy->Release_Controller(m_pController);
        m_pController = nullptr;
    }

    __super::Free();
}