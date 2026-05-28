#include "Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
	, m_fFovy(XMConvertToRadians(60.f))
	, m_fNear(0.1f)
	, m_fFar(1000.f)
	, m_vEye(_float3(0.f, 5.f, -10.f))
	, m_vAt(0.f, 0.f, 0.f)
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject(Prototype)
	, m_fFovy(Prototype.m_fFovy)
	, m_fNear(Prototype.m_fNear)
	, m_fFar(Prototype.m_fFar)
	, m_vEye(Prototype.m_vEye)
	, m_vAt(Prototype.m_vAt)
{
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize(void* pArg)
{
	if (pArg != nullptr) {
		auto pDesc = static_cast<CAMERA_DESC*>(pArg);

		m_fNear = pDesc->fNear;
		m_fFar = pDesc->fFar;
		m_fFovy = pDesc->fFovy;

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;

		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vEye), 1.f));
		m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vAt), 1.f));
	}
	else {
		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	Recalculate_ProjMatrix();
	Update_PipeLine();

	return S_OK;
}

void CCamera::Priority_Update(_float fTimeDelta)
{
	if (!m_bActive) return;

	Update_PipeLine();
}

void CCamera::Update(_float fTimeDelta)
{
	if (!m_bActive) return;
}

void CCamera::Late_Update(_float fTimeDelta)
{
	if (!m_bActive) return;
}

HRESULT CCamera::Render()
{
	return S_OK;
}

void CCamera::Deserialize(const json& j)
{
	__super::Deserialize(j);
	Recalculate_ProjMatrix();
}

void CCamera::Recalculate_ProjMatrix()
{
	D3D11_VIEWPORT vp{};
	_uint iNum = 1;
	m_pContext->RSGetViewports(&iNum, &vp);

	_float fAspect = vp.Width / vp.Height;

	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixPerspectiveFovLH(m_fFovy, fAspect, m_fNear, m_fFar));
}

void CCamera::Update_PipeLine()
{
	m_pGameInstance_Proxy->Set_Transform(D3DTS::VIEW, PROJ_TYPE::PERSPEC, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
	m_pGameInstance_Proxy->Set_Transform(D3DTS::PROJ, PROJ_TYPE::PERSPEC, XMLoadFloat4x4(&m_ProjMatrix));
}

CGameObject* CCamera::Clone(void* pArg)
{
	return nullptr;
}

void CCamera::Free()
{
	__super::Free();
}
