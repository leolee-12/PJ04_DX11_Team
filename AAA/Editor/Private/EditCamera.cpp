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
    if (m_bActive)
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
