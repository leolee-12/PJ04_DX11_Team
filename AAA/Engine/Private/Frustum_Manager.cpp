#include "Frustum_Manager.h"
#include "Culling_Util.h"

#include <cmath>

NS_BEGIN(Engine)

namespace
{
	inline _bool Is_ValidViewIndex(CULLING_VIEW eView)
	{
		return ETOUI(eView) < ETOUI(CULLING_VIEW::END);
	}

	inline _bool Is_FiniteFloat(_float fValue)
	{
		return std::isfinite(fValue);
	}

	inline _bool Is_ValidFloat3(const _float3& vValue)
	{
		return Is_FiniteFloat(vValue.x)
			&& Is_FiniteFloat(vValue.y)
			&& Is_FiniteFloat(vValue.z);
	}

	inline _bool Is_ValidBoundingBox(const BoundingBox& Bounds)
	{
		return Is_ValidFloat3(Bounds.Center)
			&& Is_ValidFloat3(Bounds.Extents)
			&& Bounds.Extents.x >= 0.f
			&& Bounds.Extents.y >= 0.f
			&& Bounds.Extents.z >= 0.f;
	}

	inline void Reset_ViewState(CFrustum_Manager::FRUSTUM_VIEW_STATE* pState)
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

CFrustum_Manager::CFrustum_Manager()
{
}

HRESULT CFrustum_Manager::Initialize()
{
	Invalidate_All();
#ifdef _DEBUG
	Reset_Stats();
#endif
	return S_OK;
}

void CFrustum_Manager::Update(const _float4x4* pMatView, const _float4x4* pMatProj)
{
#ifdef _DEBUG
	Reset_Stats();
#endif

	CULLING_VIEW_DESC MainViewDesc{};
	MainViewDesc.pView = pMatView;
	MainViewDesc.pProj = pMatProj;
	MainViewDesc.fCullMargin = 0.f;

	Update_View(CULLING_VIEW::MAIN_CAMERA, MainViewDesc);
}

_bool CFrustum_Manager::Update_View(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc)
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
	LocalFrustum.Transform(State.WorldFrustum, ViewInverse);

	State.bValid = true;

	if (Is_FiniteFloat(Desc.fCullMargin) && Desc.fCullMargin > 0.f)
		State.fCullMargin = Desc.fCullMargin;

	return true;
}

void CFrustum_Manager::Invalidate_View(CULLING_VIEW eView)
{
	if (!Is_ValidViewIndex(eView))
		return;

	Reset_ViewState(&m_ViewStates[ETOUI(eView)]);
}

void CFrustum_Manager::Invalidate_All()
{
	for (_uint i = 0; i < ETOUI(CULLING_VIEW::END); ++i)
		Reset_ViewState(&m_ViewStates[i]);
}

_bool CFrustum_Manager::Is_Valid(CULLING_VIEW eView) const
{
	if (!Is_ValidViewIndex(eView))
		return false;

	return m_ViewStates[ETOUI(eView)].bValid;
}

_bool XM_CALLCONV CFrustum_Manager::IsIn_WorldSpace(CULLING_VIEW eView, _fvector vWorldPos, _float fRange) const
{
	if (!Is_ValidViewIndex(eView))
		return true;

	const CFrustum_Manager::FRUSTUM_VIEW_STATE& State = m_ViewStates[ETOUI(eView)];
	if (!State.bValid)
		return true;

	_float3 vCenter{};
	XMStoreFloat3(&vCenter, vWorldPos);

	if (!Is_ValidFloat3(vCenter))
		return true;

	if (!Is_FiniteFloat(fRange))
		return true;

	BoundingSphere WorldSphere{};
	WorldSphere.Center = vCenter;
	WorldSphere.Radius = (fRange > 0.f) ? fRange : 0.f;

	return State.WorldFrustum.Intersects(WorldSphere);
}

_bool CFrustum_Manager::IsIn_WorldSpace_AABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const
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

	if (!Is_ValidBoundingBox(WorldBounds))
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
		CCulling_Util::Expand_AABB(&ExpandedBounds, State.fCullMargin);

	const _bool bVisible = State.WorldFrustum.Intersects(ExpandedBounds);

#ifdef _DEBUG
	if (bVisible)
		++Stats.iPassedAABB;
#endif

	return bVisible;
}

_bool CFrustum_Manager::Should_CullAABB(CULLING_VIEW eView, _bool bEnableCulling, const BoundingBox& WorldBounds) const
{
	if (!bEnableCulling)
	{
#ifdef _DEBUG
		if (Is_ValidViewIndex(eView))
			++const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]).iDisabledPolicy;
#endif
		return false;
	}

	const _bool bVisible = IsIn_WorldSpace_AABB(eView, WorldBounds);
	const _bool bCull = !bVisible;

#ifdef _DEBUG
	if (bCull && Is_ValidViewIndex(eView))
		++const_cast<FRUSTUM_CULLING_STATS&>(m_Stats[ETOUI(eView)]).iCulledAABB;
#endif

	return bCull;
}

#ifdef _DEBUG
void CFrustum_Manager::Reset_Stats()
{
	for (_uint i = 0; i < ETOUI(CULLING_VIEW::END); ++i)
		Reset_StatsSlot(&m_Stats[i]);
}

const FRUSTUM_CULLING_STATS& CFrustum_Manager::Get_Stats(CULLING_VIEW eView) const
{
	static FRUSTUM_CULLING_STATS EmptyStats{};

	if (!Is_ValidViewIndex(eView))
		return EmptyStats;

	return m_Stats[ETOUI(eView)];
}
#endif

CFrustum_Manager* CFrustum_Manager::Create()
{
	CFrustum_Manager* pInstance = new CFrustum_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Create Failed : CFrustum_Manager");
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CFrustum_Manager::Free()
{
	Invalidate_All();
}

NS_END