#include "EditCamera.h"
#include "imgui.h"

using namespace MapTool;

CEditCamera::CEditCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CEditCamera::CEditCamera(const CEditCamera& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CEditCamera::Initialize_Prototype() { return S_OK; }

HRESULT CEditCamera::Initialize(void* pArg)
{
	auto pDesc = static_cast<EDIT_CAMERA_FREE_DESC*>(pArg);

	if (nullptr != pDesc)
	{
		m_fMouseSensor = pDesc->fMouseSensor;
		m_vDefaultEye = pDesc->vEye;
		m_vDefaultAt = pDesc->vAt;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	return S_OK;
}

void CEditCamera::Priority_Update(_float fTimeDelta)
{
	if (m_bActive)
	{
		ImGuiIO& io = ImGui::GetIO();

		const _float fBaseMoveScale = 2.f;
		const _float fShiftMoveScale = io.KeyShift ? 3.f : 1.f;
		const _float fMoveTimeDelta = fTimeDelta * fBaseMoveScale * fShiftMoveScale;

		if (io.MouseDown[1])   // 우클릭 드래그 = 회전
		{
			if (io.MouseDelta.x)
				m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), io.MouseDelta.x * m_fMouseSensor * fTimeDelta);
			if (io.MouseDelta.y)
				m_pTransformCom->Turn(m_pTransformCom->Get_State(STATE::RIGHT), io.MouseDelta.y * m_fMouseSensor * fTimeDelta);
		}

		if (io.MouseWheel != 0.f)   // 휠 = 전후진
			m_pTransformCom->Go_Straight(fMoveTimeDelta * io.MouseWheel);

		if (io.MouseDown[1])        // 우클릭 중 WASD
		{
			if (ImGui::IsKeyDown(ImGuiKey_W)) m_pTransformCom->Go_Straight(fMoveTimeDelta);
			if (ImGui::IsKeyDown(ImGuiKey_S)) m_pTransformCom->Go_Backward(fMoveTimeDelta);
			if (ImGui::IsKeyDown(ImGuiKey_A)) m_pTransformCom->Go_Left(fMoveTimeDelta);
			if (ImGui::IsKeyDown(ImGuiKey_D)) m_pTransformCom->Go_Right(fMoveTimeDelta);
		}
	}
	__super::Priority_Update(fTimeDelta);
}

void CEditCamera::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CEditCamera::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CEditCamera::Render() { return E_NOTIMPL; }

void CEditCamera::Reset_Rotation()
{
	if (nullptr == m_pTransformCom)
		return;

	_vector vDefaultLook = XMLoadFloat3(&m_vDefaultAt) - XMLoadFloat3(&m_vDefaultEye);
	if (0.f >= XMVectorGetX(XMVector3LengthSq(vDefaultLook)))
		vDefaultLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	const _vector vTarget = XMVectorSetW(vPosition + XMVector3Normalize(vDefaultLook), 1.f);
	m_pTransformCom->LookAt(vTarget);
}

void CEditCamera::Jump_Local(_float fForwardDistance, _float fRightDistance)
{
	if (nullptr == m_pTransformCom)
		return;

	_vector vMove = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)) * fForwardDistance;
	vMove += XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT)) * fRightDistance;

	if (0.f >= XMVectorGetX(XMVector3LengthSq(vMove)))
		return;

	const _vector vNextPosition =
		XMVectorSetW(m_pTransformCom->Get_State(STATE::POSITION) + vMove, 1.f);

	m_pTransformCom->Set_State(STATE::POSITION, vNextPosition);
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