#include "Culling_Manager.h"
#include "Geometry_Utils.h"
#include "Math_Utils.h"
#include "Profiler_Manager.h"
#include "GameInstance.h"

NS_BEGIN(Engine)

namespace
{
	constexpr _float INVALID_SURFACE_DISTANCE = -1.f;

	inline _bool Is_ValidViewIndex(CULLING_VIEW eView)
	{
		return ETOUI(eView) < ETOUI(CULLING_VIEW::END);
	}

	inline void Store_InsidePlane(_float4* pOutPlane, _fvector vOutwardPlane)
	{
		if (nullptr == pOutPlane)
			return;

		XMStoreFloat4(pOutPlane, XMPlaneNormalize(XMVectorNegate(vOutwardPlane)));
	}

	inline void Reset_ViewState(CCulling_Manager::FRUSTUM_VIEW_STATE* pState)
	{
		if (nullptr == pState)
			return;

		pState->WorldFrustum = {};

		for (_uint i = 0; i < ETOUI(CCulling_Manager::CULLING_PLANE::COUNT); ++i)
			pState->WorldPlanes[i] = {};

		pState->bValid = false;
		pState->fCullMargin = 0.f;
	}

#ifdef _DEBUG
	inline void Debug_ValidatePlaneCache(const CCulling_Manager::FRUSTUM_VIEW_STATE& State)
	{
		const _float fInsideDistance = (State.WorldFrustum.Near + State.WorldFrustum.Far) * 0.5f;
		if (!MathUtils::Is_FiniteFloat(fInsideDistance) || fInsideDistance <= 0.f)
			return;

		const _vector vForward = XMVector3Rotate(
			XMVectorSet(0.f, 0.f, 1.f, 0.f),
			XMLoadFloat4(&State.WorldFrustum.Orientation));
		const _vector vInsidePoint = XMVectorMultiplyAdd(
			vForward,
			XMVectorReplicate(fInsideDistance),
			XMLoadFloat3(&State.WorldFrustum.Origin));

		_bool bInsidePlanes = true;
		for (_uint i = 0; i < ETOUI(CCulling_Manager::CULLING_PLANE::COUNT); ++i)
		{
			const _float fSignedDistance = XMVectorGetX(
				XMPlaneDotCoord(XMLoadFloat4(&State.WorldPlanes[i]), vInsidePoint));
			bInsidePlanes = bInsidePlanes && fSignedDistance >= -Helper::fEpsilon;
		}

		_float3 vInsidePointFloat{};
		XMStoreFloat3(&vInsidePointFloat, vInsidePoint);

		BoundingSphere InsideBounds{};
		InsideBounds.Center = vInsidePointFloat;

		assert(bInsidePlanes == State.WorldFrustum.Intersects(InsideBounds));
	}
#endif
}

CCulling_Manager::CCulling_Manager()
	: m_pProxy(CGameInstance::GetProxy())
{
}

HRESULT CCulling_Manager::Initialize()
{
	Invalidate_All();

	return S_OK;
}

void CCulling_Manager::Update()
{
	// Main_Camera
	CULLING_VIEW_DESC MainViewDesc{};
	MainViewDesc.pView = m_pProxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
	MainViewDesc.pProj = m_pProxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
	MainViewDesc.fCullMargin = 5.f;
	Update_View(CULLING_VIEW::MAIN_CAMERA, MainViewDesc);

	// Shadow_Dir
	CULLING_VIEW_DESC ShadowViewDesc{};
	ShadowViewDesc.pView = m_pProxy->Get_Shadow_Transform(D3DTS::VIEW);
	ShadowViewDesc.pProj = m_pProxy->Get_Shadow_Transform(D3DTS::PROJ);
	ShadowViewDesc.fCullMargin = 30.f;
	Update_View(CULLING_VIEW::SHADOW_DIR, ShadowViewDesc);
}

_bool CCulling_Manager::Update_View(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc)
{
	if (!Is_ValidViewIndex(eView))
		return false;

	FRUSTUM_VIEW_STATE& State = m_ViewStates[ETOUI(eView)];
	Reset_ViewState(&State);

	if (nullptr == Desc.pView || nullptr == Desc.pProj)
		return false;

	BoundingFrustum LocalFrustum{};
	BoundingFrustum::CreateFromMatrix(LocalFrustum, XMLoadFloat4x4(Desc.pProj));

	const _matrix ViewInverse = XMMatrixInverse(nullptr, XMLoadFloat4x4(Desc.pView));
	float det = XMVectorGetX(XMMatrixDeterminant(ViewInverse));
	if (!(fabsf(det) > 1e-8f))      // NaN(비교 false) 또는 ~0 -> 무효
	{
		State.bValid = false;       // IsIn_*가 true(보임) 반환 = 컬링 패스
		return false;
	}
	LocalFrustum.Transform(State.WorldFrustum, ViewInverse);

	XMStoreFloat4(&State.WorldFrustum.Orientation,
		XMQuaternionNormalize(XMLoadFloat4(&State.WorldFrustum.Orientation)));

	_vector vNearPlane = {};
	_vector vFarPlane = {};
	_vector vRightPlane = {};
	_vector vLeftPlane = {};
	_vector vTopPlane = {};
	_vector vBottomPlane = {};
	State.WorldFrustum.GetPlanes(&vNearPlane, &vFarPlane, &vRightPlane, &vLeftPlane, &vTopPlane, &vBottomPlane);

	Store_InsidePlane(&State.WorldPlanes[ETOUI(CULLING_PLANE::LEFT)], vLeftPlane);
	Store_InsidePlane(&State.WorldPlanes[ETOUI(CULLING_PLANE::RIGHT)], vRightPlane);
	Store_InsidePlane(&State.WorldPlanes[ETOUI(CULLING_PLANE::TOP)], vTopPlane);
	Store_InsidePlane(&State.WorldPlanes[ETOUI(CULLING_PLANE::BOTTOM)], vBottomPlane);
	Store_InsidePlane(&State.WorldPlanes[ETOUI(CULLING_PLANE::NEAR_PLANE)], vNearPlane);
	Store_InsidePlane(&State.WorldPlanes[ETOUI(CULLING_PLANE::FAR_PLANE)], vFarPlane);

#ifdef _DEBUG
	Debug_ValidatePlaneCache(State);
#endif

	State.bValid = true;

	if (MathUtils::Is_FiniteFloat(Desc.fCullMargin) && Desc.fCullMargin > 0.f)
		State.fCullMargin = Desc.fCullMargin;

	return true;
}

void CCulling_Manager::Invalidate_All()
{
	for (_uint i = 0; i < ETOUI(CULLING_VIEW::END); ++i)
		Reset_ViewState(&m_ViewStates[i]);
}

_bool CCulling_Manager::Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const
{
	if (!Is_ValidViewIndex(eView))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);
		return false;
	}

	const FRUSTUM_VIEW_STATE& State = m_ViewStates[ETOUI(eView)];
	if (!State.bValid)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);
		return false;
	}

	if (!GeometryUtils::Is_ValidAABB(WorldBounds))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_BOUNDS, 1);
		return false;
	}

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_TESTED, 1);

	BoundingBox ExpandedBounds = WorldBounds;
	if (State.fCullMargin > 0.f)
		GeometryUtils::Expand_AABB(&ExpandedBounds, State.fCullMargin);

	const _bool bVisible = State.WorldFrustum.Intersects(ExpandedBounds);
	const _bool bCull = !bVisible;

	PROFILE_COUNTER_ADD(bVisible ? EPROFILE_COUNTER::FRUSTUM_VISIBLE : EPROFILE_COUNTER::FRUSTUM_CULLED, 1);

	return bCull;
}

_float CCulling_Manager::Compute_SurfaceDistance(const BoundingSphere& WorldBounds) const
{
	if (nullptr == m_pProxy)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_CAMERA, 1);
		return INVALID_SURFACE_DISTANCE;
	}

	const _float4* pCamPos = m_pProxy->Get_CamPosition();
	if (nullptr == pCamPos
		|| !MathUtils::Is_FiniteFloat(pCamPos->x)
		|| !MathUtils::Is_FiniteFloat(pCamPos->y)
		|| !MathUtils::Is_FiniteFloat(pCamPos->z))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_CAMERA, 1);
		return INVALID_SURFACE_DISTANCE;
	}

	if (!MathUtils::Is_ValidFloat3(WorldBounds.Center)
		|| !MathUtils::Is_FiniteFloat(WorldBounds.Radius)
		|| WorldBounds.Radius < 0.f)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_BOUNDS, 1);
		return INVALID_SURFACE_DISTANCE;
	}

	const _vector vCam = XMLoadFloat4(pCamPos);
	const _vector vCenter = XMLoadFloat3(&WorldBounds.Center);
	const _float fCenterDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vCenter, vCam)));

	if (!MathUtils::Is_FiniteFloat(fCenterDistance))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_BOUNDS, 1);
		return INVALID_SURFACE_DISTANCE;
	}

	return (fCenterDistance > WorldBounds.Radius)
		? (fCenterDistance - WorldBounds.Radius)
		: 0.f;
}

CULLING_FADE_RESULT CCulling_Manager::Evaluate_FrustumFadeAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds, _uint iPlaneMask) const
{
	CULLING_FADE_RESULT Result{};

	if (!Is_ValidViewIndex(eView))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);
		return Result;
	}

	const FRUSTUM_VIEW_STATE& State = m_ViewStates[ETOUI(eView)];
	if (!State.bValid)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);
		return Result;
	}

	if (!GeometryUtils::Is_ValidAABB(WorldBounds))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_BOUNDS, 1);
		return Result;
	}

	const _uint iValidPlaneMask = (1u << ETOUI(CULLING_PLANE::COUNT)) - 1u;
	iPlaneMask &= iValidPlaneMask;

	if (0u == iPlaneMask)
		return Result;

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_TESTED, 1);

	BoundingBox ExpandedBounds = WorldBounds;
	if (State.fCullMargin > 0.f)
		GeometryUtils::Expand_AABB(&ExpandedBounds, State.fCullMargin);

	const _vector vBoundsCenter = XMLoadFloat3(&WorldBounds.Center);
	_float fNearestSupport = FLT_MAX;
	_float fNearestExpandedSupport = FLT_MAX;

	for (_uint i = 0; i < ETOUI(CULLING_PLANE::COUNT); ++i)
	{
		if (0u == (iPlaneMask & (1u << i)))
			continue;

		const _float4& Plane = State.WorldPlanes[i];
		const _float fCenterDistance = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&Plane), vBoundsCenter));
		const _float fProjectedRadius =
			fabsf(Plane.x) * WorldBounds.Extents.x +
			fabsf(Plane.y) * WorldBounds.Extents.y +
			fabsf(Plane.z) * WorldBounds.Extents.z;
		const _float fExpandedProjectedRadius =
			fabsf(Plane.x) * ExpandedBounds.Extents.x +
			fabsf(Plane.y) * ExpandedBounds.Extents.y +
			fabsf(Plane.z) * ExpandedBounds.Extents.z;
		const _float fSupportDistance = fCenterDistance + fProjectedRadius;
		const _float fExpandedSupportDistance = fCenterDistance + fExpandedProjectedRadius;

		if (!MathUtils::Is_FiniteFloat(fSupportDistance) || !MathUtils::Is_FiniteFloat(fExpandedSupportDistance))
		{
			PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);
			return CULLING_FADE_RESULT{};
		}

		fNearestSupport = min(fNearestSupport, fSupportDistance);
		fNearestExpandedSupport = min(fNearestExpandedSupport, fExpandedSupportDistance);

		if (fSupportDistance >= 0.f || fExpandedSupportDistance <= 0.f)
			continue;

		const _float fMarginSupport = fExpandedSupportDistance - fSupportDistance;
		_float fLinear = fExpandedSupportDistance / fMarginSupport;
		fLinear = max(0.f, min(fLinear, 1.f));
		const _float fSmooth = fLinear * fLinear * (3.f - 2.f * fLinear);

		Result.fDissolve = max(Result.fDissolve, 1.f - fSmooth);
	}

	Result.fBoundaryDistance = fNearestSupport;

	if (fNearestExpandedSupport <= 0.f)
	{
		Result.bCulled = true;
		Result.fDissolve = 1.f;

		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_CULLED, 1);

#ifdef _DEBUG
		const _float fBoundsScale = max(1.f,
			fabsf(ExpandedBounds.Center.x) + fabsf(ExpandedBounds.Center.y) + fabsf(ExpandedBounds.Center.z)
			+ ExpandedBounds.Extents.x + ExpandedBounds.Extents.y + ExpandedBounds.Extents.z);
		if (fNearestExpandedSupport < -(fBoundsScale * 1e-4f))
			assert(!State.WorldFrustum.Intersects(ExpandedBounds));
#endif

		return Result;
	}

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_VISIBLE, 1);
	return Result;
}

CULLING_FADE_RESULT CCulling_Manager::Evaluate_DistanceFade(const BoundingSphere& WorldBounds, _float fCullDistance, _float fFadeWidth) const
{
	const _float fSurfaceDistance = Compute_SurfaceDistance(WorldBounds);

	if (!MathUtils::Is_FiniteFloat(fSurfaceDistance) || fSurfaceDistance < 0.f)
		return {};

	return Evaluate_DistanceFade(fSurfaceDistance, fCullDistance, fFadeWidth);
}

CULLING_FADE_RESULT CCulling_Manager::Evaluate_DistanceFade(
	_float fSurfaceDistance,
	_float fCullDistance,
	_float fFadeWidth) const
{
	CULLING_FADE_RESULT Result{};

	if (!MathUtils::Is_FiniteFloat(fSurfaceDistance) || fSurfaceDistance < 0.f)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_BOUNDS, 1);
		return Result;
	}

	if (!MathUtils::Is_FiniteFloat(fCullDistance) || fCullDistance < 0.f
		|| !MathUtils::Is_FiniteFloat(fFadeWidth))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_DISTANCE, 1);
		return Result;
	}

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_TESTED, 1);

	Result.fBoundaryDistance = fSurfaceDistance;

	if (fSurfaceDistance >= fCullDistance)
	{
		Result.bCulled = true;
		Result.fDissolve = 1.f;

		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_CULLED, 1);
		return Result;
	}

	if (fFadeWidth <= Helper::fEpsilon)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_VISIBLE, 1);
		return Result;
	}

	const _float fFadeStart = max(fCullDistance - fFadeWidth, 0.f);
	if (fSurfaceDistance <= fFadeStart)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_VISIBLE, 1);
		return Result;
	}

	const _float fFadeRange = fCullDistance - fFadeStart;
	_float fLinear = (fSurfaceDistance - fFadeStart) / fFadeRange;
	fLinear = max(0.f, min(fLinear, 1.f));

	Result.fDissolve = fLinear * fLinear * (3.f - 2.f * fLinear);

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_VISIBLE, 1);
	return Result;
}

CCulling_Manager* CCulling_Manager::Create()
{
	CCulling_Manager* pInstance = new CCulling_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Create Failed : CCulling_Manager");
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CCulling_Manager::Free()
{
	Invalidate_All();

	Safe_Release(m_pProxy);

	__super::Free();
}

NS_END