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
    auto        pDesc = static_cast<EDIT_CAMERA_FREE_DESC*>(pArg);

    m_fMouseSensor = pDesc->fMouseSensor;


    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CEditCamera::Priority_Update(_float fTimeDelta)
{
    //if (m_bActive)
    //{
    //    ImGuiIO& io = ImGui::GetIO();
    //
    //    // 우클릭 드래그 → 회전
    //    if (io.MouseDown[1])
    //    {
    //        // io.MouseDelta.x, io.MouseDelta.y 로 Yaw/Pitch
    //        if (io.MouseDelta.x)
    //        {
    //            m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), io.MouseDelta.x * m_fMouseSensor * fTimeDelta);
    //        }
    //        if (io.MouseDelta.y)
    //        {
    //            m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), io.MouseDelta.y * m_fMouseSensor * fTimeDelta);
    //        }
    //    }
    //
    //    // 휠 → 줌
    //    if (io.MouseWheel != 0.f)
    //    {
    //        m_pTransformCom->Go_Straight(fTimeDelta * io.MouseWheel);
    //    }
    //
    //    // WASD (우클릭 중일 때만)
    //    if (io.MouseDown[1])
    //    {
    //        if (ImGui::IsKeyDown(ImGuiKey_W)) {
    //            m_pTransformCom->Go_Straight(fTimeDelta);
    //        }
    //        if (ImGui::IsKeyDown(ImGuiKey_S)) {
    //            m_pTransformCom->Go_Backward(fTimeDelta);
    //        }
    //        if (ImGui::IsKeyDown(ImGuiKey_A)) {
    //            m_pTransformCom->Go_Left(fTimeDelta);
    //        }
    //        if (ImGui::IsKeyDown(ImGuiKey_D)) {
    //            m_pTransformCom->Go_Right(fTimeDelta);
    //        }
    //    }
    //}
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
