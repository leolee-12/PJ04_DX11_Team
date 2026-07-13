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

	inline void Reset_ViewState(CCulling_Manager::FRUSTUM_VIEW_STATE* pState)
	{
		if (nullptr == pState)
			return;

		pState->WorldFrustum = {};
		pState->bValid = false;
		pState->fCullMargin = 0.f;
	}

#ifdef _DEBUG
	inline void Reset_StatsSlot(FRUSTUM_CULLING_STATS* pStats)
	{
		if (nullptr == pStats)
			return;

		*pStats = {};
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
#ifdef _DEBUG
	Reset_Stats();
#endif
	return S_OK;
}

void CCulling_Manager::Update()
{
#ifdef _DEBUG
	Reset_Stats();
#endif

	// Main_Camera
	CULLING_VIEW_DESC MainViewDesc{};
	MainViewDesc.pView = m_pProxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
	MainViewDesc.pProj = m_pProxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
	MainViewDesc.fCullMargin = 12.f;
	Update_View(CULLING_VIEW::MAIN_CAMERA, MainViewDesc);

	// Shadow_Dir
	CULLING_VIEW_DESC ShadowViewDesc{};
	ShadowViewDesc.pView = m_pProxy->Get_Shadow_Transform(D3DTS::VIEW);
	ShadowViewDesc.pProj = m_pProxy->Get_Shadow_Transform(D3DTS::PROJ);
	ShadowViewDesc.fCullMargin = 12.f;
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

	State.bValid = true;

	if (MathUtils::Is_FiniteFloat(Desc.fCullMargin) && Desc.fCullMargin > 0.f)
		State.fCullMargin = Desc.fCullMargin;

	return true;
}

void CCulling_Manager::Invalidate_View(CULLING_VIEW eView)
{
	if (!Is_ValidViewIndex(eView))
		return;

	Reset_ViewState(&m_ViewStates[ETOUI(eView)]);
}

void CCulling_Manager::Invalidate_All()
{
	for (_uint i = 0; i < ETOUI(CULLING_VIEW::END); ++i)
		Reset_ViewState(&m_ViewStates[i]);
}

_bool CCulling_Manager::Is_Valid(CULLING_VIEW eView) const
{
	if (!Is_ValidViewIndex(eView))
		return false;

	return m_ViewStates[ETOUI(eView)].bValid;
}

_bool XM_CALLCONV CCulling_Manager::IsIn_WorldSpace(CULLING_VIEW eView, _fvector vWorldPos, _float fRange) const
{
	if (!Is_ValidViewIndex(eView))
		return true;

	const CCulling_Manager::FRUSTUM_VIEW_STATE& State = m_ViewStates[ETOUI(eView)];
	if (!State.bValid)
		return true;

	_float3 vCenter{};
	XMStoreFloat3(&vCenter, vWorldPos);

	if (!MathUtils::Is_ValidFloat3(vCenter))
		return true;

	if (!MathUtils::Is_FiniteFloat(fRange))
		return true;

	BoundingSphere WorldSphere{};
	WorldSphere.Center = vCenter;
	WorldSphere.Radius = (fRange > 0.f) ? fRange : 0.f;

	return State.WorldFrustum.Intersects(WorldSphere);
}

_bool CCulling_Manager::IsIn_WorldSpace_AABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const
{
	if (!Is_ValidViewIndex(eView))
		return true;

	const FRUSTUM_VIEW_STATE& State = m_ViewStates[ETOUI(eView)];

#ifdef _DEBUG
	FRUSTUM_CULLING_STATS& Stats = const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]);
#endif

	if (!State.bValid)
	{
#ifdef _DEBUG
		++Stats.iInvalidViewFailOpen;
#endif
		return true;
	}

	if (!GeometryUtils::Is_ValidAABB(WorldBounds))
	{
#ifdef _DEBUG
		++Stats.iInvalidBoundsFailOpen;
#endif
		return true;
	}

#ifdef _DEBUG
	++Stats.iTestedAABB;
#endif

	BoundingBox ExpandedBounds = WorldBounds;
	if (State.fCullMargin > 0.f)
		GeometryUtils::Expand_AABB(&ExpandedBounds, State.fCullMargin);

	const _bool bVisible = State.WorldFrustum.Intersects(ExpandedBounds);

#ifdef _DEBUG
	if (bVisible)
		++Stats.iPassedAABB;
#endif

	return bVisible;
}

_bool CCulling_Manager::Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const
{
	if (!m_bEnableFrustumCulling)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_DISABLED_POLICY, 1);

#ifdef _DEBUG
		if (Is_ValidViewIndex(eView))
			++const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]).iDisabledPolicy;
#endif
		return false;
	}

	if (!Is_ValidViewIndex(eView))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);
		return false;
	}

	if (!m_ViewStates[ETOUI(eView)].bValid)
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW, 1);

#ifdef _DEBUG
		++const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]).iInvalidViewFailOpen;
#endif
		return false;
	}

	if (!GeometryUtils::Is_ValidAABB(WorldBounds))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_BOUNDS, 1);

#ifdef _DEBUG
		++const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]).iInvalidBoundsFailOpen;
#endif
		return false;
	}

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::FRUSTUM_TESTED, 1);

	const _bool bVisible = IsIn_WorldSpace_AABB(eView, WorldBounds);
	const _bool bCull = !bVisible;

	PROFILE_COUNTER_ADD(
		bVisible ? EPROFILE_COUNTER::FRUSTUM_VISIBLE : EPROFILE_COUNTER::FRUSTUM_CULLED,
		1);

#ifdef _DEBUG
	if (bCull)
		++const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]).iCulledAABB;
#endif

	return bCull;
}

_bool CCulling_Manager::Should_CullByDistance(const BoundingBox& WorldBounds, _float fCullDistance) const
{
	if (!GeometryUtils::Is_ValidAABB(WorldBounds))
	{
		PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_BOUNDS, 1);
		return false;
	}

	const _float fRadius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&WorldBounds.Extents)));
	const BoundingSphere WorldSphere(WorldBounds.Center, fRadius);

	return Evaluate_DistanceFade(WorldSphere, fCullDistance, 0.f).bCulled;
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

CULLING_FADE_RESULT CCulling_Manager::Evaluate_DistanceFade(
	const BoundingSphere& WorldBounds,
	_float fCullDistance,
	_float fFadeWidth) const
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

	_float fLinear = (fSurfaceDistance - fFadeStart) / fFadeWidth;
	fLinear = max(0.f, min(fLinear, 1.f));

	Result.fDissolve = fLinear * fLinear * (3.f - 2.f * fLinear);

	PROFILE_COUNTER_ADD(EPROFILE_COUNTER::DISTANCE_VISIBLE, 1);
	return Result;
}

#ifdef _DEBUG
void CCulling_Manager::Reset_Stats()
{
	for (_uint i = 0; i < ETOUI(CULLING_VIEW::END); ++i)
		Reset_StatsSlot(&m_Stats[i]);
}

const FRUSTUM_CULLING_STATS& CCulling_Manager::Get_Stats(CULLING_VIEW eView) const
{
	static FRUSTUM_CULLING_STATS EmptyStats{};

	if (!Is_ValidViewIndex(eView))
		return EmptyStats;

	return m_Stats[ETOUI(eView)];
}
#endif

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