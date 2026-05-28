#include "Camera_Free.h"
#include "GameInstance.h"

CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCamera { pDevice, pContext }
{
}

CCamera_Free::CCamera_Free(const CCamera_Free& Prototype)
    : CCamera(Prototype)
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    //test
    m_bActive = true;

    return S_OK;
}

void CCamera_Free::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    /*_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
    _vector vRight = XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));
    _vector vUp = XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP));

    _float fSpeed = 10.f * fTimeDelta;

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W)) vPos += vLook * fSpeed;
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S)) vPos -= vLook * fSpeed;
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D)) vPos += vRight * fSpeed;
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A)) vPos -= vRight * fSpeed;
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_E)) vPos += vUp * fSpeed;
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_Q)) vPos -= vUp * fSpeed;

    m_pTransformCom->Set_State(STATE::POSITION, vPos);*/

    __super::Priority_Update(fTimeDelta);
}

void CCamera_Free::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    __super::Update(fTimeDelta);
}

void CCamera_Free::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    __super::Late_Update(fTimeDelta);
}

HRESULT CCamera_Free::Render()
{
    return S_OK;
}

HRESULT CCamera_Free::Ready_Events()
{
    Subscribe_Event(TEXT("Enter_View"), [this](void* pData) {
        _int iView = *static_cast<_int*>(pData);
        m_bActive = (iView == ETOI(LOBBY_VIEW::STORAGE));
        });

    return S_OK;
}

CCamera_Free* CCamera_Free::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCamera_Free* pInstance = new CCamera_Free(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CCamera_Free");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCamera_Free::Clone(void* pArg)
{
    CCamera_Free* pInstance = new CCamera_Free(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CCamera_Free");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCamera_Free::Free()
{
    __super::Free();
}
