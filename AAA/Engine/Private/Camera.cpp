#include "Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
	, m_fFovy(XMConvertToRadians(60.f))
	, m_fNear(0.1f)
	, m_fFar(1000.f)
	, m_vEye(_float3(0.f, 5.f, -10.f))
	, m_vAt(0.f, 0.f, 0.f)
	, m_fShadowRange(80.f)     
	, m_fShadowPadding(20.f)
	, m_iShadowRes(g_iShadowMapSize)
	, m_bDriveShadowFit(true)
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject(Prototype)
	, m_fFovy(Prototype.m_fFovy)
	, m_fNear(Prototype.m_fNear)
	, m_fFar(Prototype.m_fFar)
	, m_vEye(Prototype.m_vEye)
	, m_vAt(Prototype.m_vAt)
	, m_fShadowRange(Prototype.m_fShadowRange)        
	, m_fShadowPadding(Prototype.m_fShadowPadding)    
	, m_iShadowRes(Prototype.m_iShadowRes)            
	, m_bDriveShadowFit(Prototype.m_bDriveShadowFit)
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

		pDesc->fRotationPerSec = 180.f;
		pDesc->fSpeedPerSec = 20.f;

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
	Update_ShadowFit();
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

void CCamera::Deserialize_Internal(const json& j)
{
	__super::Deserialize_Internal(j);
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

SHADOW_LIGHT_DESC CCamera::Make_CameraFit_Shadow(const _float4& vLightDir, _float fRange, _float fPadding, _uint iRes) const
{
	// 1) 라이브 뷰행렬 (m_vEye/m_vAt는 초기값이라 사용 금지 → 트랜스폼 월드의 역)
	XMMATRIX matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	XMMATRIX matView = XMMatrixInverse(nullptr, matWorld);
	XMMATRIX matProj = XMLoadFloat4x4(&m_ProjMatrix);
	XMMATRIX matInvVP = XMMatrixInverse(nullptr, matView * matProj);

	// 2) 그림자 구간 [near, fRange]의 far를 NDC z로 (LH 원근)
	float fDenom = fRange * (m_fFar - m_fNear);
	float zFarNdc = (fDenom > 1e-4f) ? (m_fFar * (fRange - m_fNear)) / fDenom : 1.f;
	zFarNdc = (zFarNdc < 0.f) ? 0.f : (zFarNdc > 1.f ? 1.f : zFarNdc);

	// 3) 8코너(월드) + 중심
	const float xs[4] = { -1.f, 1.f, -1.f, 1.f };
	const float ys[4] = { -1.f, -1.f, 1.f, 1.f };
	XMVECTOR vCorner[8];
	XMVECTOR vCenter = XMVectorZero();
	for (int i = 0; i < 4; ++i)
	{
		vCorner[i] = XMVector3TransformCoord(XMVectorSet(xs[i], ys[i], 0.f, 1.f), matInvVP);
		vCorner[i + 4] = XMVector3TransformCoord(XMVectorSet(xs[i], ys[i], zFarNdc, 1.f), matInvVP);
		vCenter = vCenter + vCorner[i] + vCorner[i + 4];
	}
	vCenter = vCenter / 8.f;

	// 4) 회전 불변 반경(바운딩 스피어) → 폭/높이 고정
	float fRadius = 0.f;
	for (int i = 0; i < 8; ++i)
		fRadius = fmaxf(fRadius, XMVectorGetX(XMVector3Length(vCorner[i] - vCenter)));
	fRadius = ceilf(fRadius * 16.f) / 16.f;   // 미세 떨림 억제용 양자화
	if (fRadius > m_fShadowMaxRadius)
		fRadius = m_fShadowMaxRadius;

	// 5) 라이트 축 (Shadow_Dir 내부 up=(0,1,0)과 반드시 일치)
	XMVECTOR L = XMVector3Normalize(XMVectorSet(vLightDir.x, vLightDir.y, vLightDir.z, 0.f));
	XMVECTOR vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR vRight = XMVector3Normalize(XMVector3Cross(vUp, L));
	XMVECTOR vTrueUp = XMVector3Cross(L, vRight);

	// 6) 텍셀 스냅: 중심을 (right, up) 텍셀 격자에 정렬
	float fRes = (iRes < 1u) ? 1.f : (float)iRes;
	float fWPT = (2.f * fRadius) / fRes;
	float cr = floorf(XMVectorGetX(XMVector3Dot(vCenter, vRight)) / fWPT) * fWPT;
	float cu = floorf(XMVectorGetX(XMVector3Dot(vCenter, vTrueUp)) / fWPT) * fWPT;
	float cl = XMVectorGetX(XMVector3Dot(vCenter, L));
	vCenter = vRight * cr + vTrueUp * cu + L * cl;

	// 7) eye를 빛 반대로 빼 캐스터 포함 → desc 구성
	XMVECTOR vEye = vCenter - L * (fRadius + fPadding);

	SHADOW_LIGHT_DESC desc{};
	XMStoreFloat4(&desc.vEye, XMVectorSetW(vEye, 1.f));
	XMStoreFloat4(&desc.vAt, XMVectorSetW(vCenter, 1.f));
	desc.fWidth = 2.f * fRadius;
	desc.fHeight = 2.f * fRadius;
	desc.fNear = 0.05f;
	desc.fFar = 2.f * fRadius + 2.f * fPadding;
	return desc;
}

void CCamera::Update_PipeLine()
{
	m_pGameInstance_Proxy->Set_Transform(D3DTS::VIEW, PROJ_TYPE::PERSPEC, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
	m_pGameInstance_Proxy->Set_Transform(D3DTS::PROJ, PROJ_TYPE::PERSPEC, XMLoadFloat4x4(&m_ProjMatrix));
}

void CCamera::Update_ShadowFit()
{
	if (!m_bDriveShadowFit) return;

	const LIGHT_DESC* pDir = nullptr;
	for (_uint i = 0; ; ++i)
	{
		const LIGHT_DESC* p = m_pGameInstance_Proxy->Get_LightDesc(i);
		if (nullptr == p) break;
		if (LIGHT::DIRECTIONAL == p->eType) { pDir = p; break; }
	}
	if (nullptr == pDir) return;

	_float4 vShadowDir = _float4(m_vShadowDir.x, m_vShadowDir.y, m_vShadowDir.z, 0.f);
	SHADOW_LIGHT_DESC desc = Make_CameraFit_Shadow(
		vShadowDir, m_fShadowRange, m_fShadowPadding, m_iShadowRes);
	m_pGameInstance_Proxy->Update_ShadowLight(desc);
}

CGameObject* CCamera::Clone(void* pArg)
{
	return nullptr;
}

void CCamera::Free()
{
	__super::Free();
}
