#include "EditCamera.h"
#include "imgui.h"

CEditCamera::CEditCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CEditCamera::CEditCamera(const CEditCamera& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CEditCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEditCamera::Initialize(void* pArg)
{
    auto pDesc = static_cast<EDIT_CAMERA_FREE_DESC*>(pArg);
    m_fMouseSensor = pDesc->fMouseSensor;
    m_fNearKeep = pDesc->fNear;
    m_fFarKeep = pDesc->fFar;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
    return S_OK;
}

void CEditCamera::Priority_Update(_float fTimeDelta)
{
    if (m_bPreview)
    {
        _vector vEye = XMLoadFloat3(&m_vPrevEye);
        _vector vLook = XMVector3Normalize(XMLoadFloat3(&m_vPrevFwd));

        _vector vUpRef = XMLoadFloat3(&m_vPrevUp);
        _vector vRight = XMVector3Cross(vUpRef, vLook);
        if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-6f)        // look이 up과 평행 -> 폴백
            vRight = XMVector3Cross(XMVectorSet(0.f, 0.f, 1.f, 0.f), vLook);
        vRight = XMVector3Normalize(vRight);
        _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vEye, 1.f));
        m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSetW(vRight, 0.f));
        m_pTransformCom->Set_State(STATE::UP, XMVectorSetW(vUp, 0.f));
        m_pTransformCom->Set_State(STATE::LOOK, XMVectorSetW(vLook, 0.f));

        _float fAspect = (_float)g_iWinSizeX / (_float)g_iWinSizeY;
        XMStoreFloat4x4(&m_ProjMatrix,
            XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fPrevFov), fAspect, m_fNearKeep, m_fFarKeep));
    }
    else if (m_bActive)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.MouseDown[1])
        {
            _float fSpeed = 1.f;
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) fSpeed = 10.f;
            if (io.MouseDelta.x != 0.f) m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), io.MouseDelta.x *
                m_fMouseSensor * fTimeDelta);
            if (io.MouseDelta.y != 0.f) m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT),
                io.MouseDelta.y * m_fMouseSensor * fTimeDelta);
            if (ImGui::IsKeyDown(ImGuiKey_W)) m_pTransformCom->Go_Straight(fTimeDelta * fSpeed);
            if (ImGui::IsKeyDown(ImGuiKey_S)) m_pTransformCom->Go_Backward(fTimeDelta * fSpeed);
            if (ImGui::IsKeyDown(ImGuiKey_A)) m_pTransformCom->Go_Left(fTimeDelta * fSpeed);
            if (ImGui::IsKeyDown(ImGuiKey_D)) m_pTransformCom->Go_Right(fTimeDelta * fSpeed);
        }
    }

    __super::Priority_Update(fTimeDelta);
}

void CEditCamera::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CEditCamera::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CEditCamera::Render()
{
	return E_NOTIMPL;
}

void CEditCamera::Set_PreviewMode(_bool b)
{
    m_bPreview = b;
    if (!b)
        Recalculate_ProjMatrix();
}

CEditCamera* CEditCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEditCamera* pInstance = new CEditCamera(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEditCamera");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEditCamera::Clone(void* pArg)
{
    CEditCamera* pInstance = new CEditCamera(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Clone : CEditCamera");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEditCamera::Free()
{
    __super::Free();
}
