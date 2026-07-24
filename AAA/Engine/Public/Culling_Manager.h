#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;

class ENGINE_DLL CCulling_Manager final : public CBase
{
public:
	enum class CULLING_PLANE : _uint { LEFT, RIGHT, TOP, BOTTOM, NEAR_PLANE, FAR_PLANE, COUNT };

	struct FRUSTUM_VIEW_STATE
	{
		BoundingFrustum		WorldFrustum = {};
		_float4				WorldPlanes[ETOUI(CULLING_PLANE::COUNT)] = {};
		_bool				bValid = { false };
		_float				fCullMargin = { 0.f };
	};

	static constexpr _uint CULLING_PLANE_MASK_MAIN_SIDE =
		(1u << ETOUI(CULLING_PLANE::LEFT)) | (1u << ETOUI(CULLING_PLANE::RIGHT))
		| (1u << ETOUI(CULLING_PLANE::TOP)) | (1u << ETOUI(CULLING_PLANE::BOTTOM));

private:
	CCulling_Manager();
	virtual ~CCulling_Manager() = default;

public:
	HRESULT Initialize();
	void	Update();

	_bool	Update_View(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc);

	_bool	Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const;

	CULLING_FADE_RESULT Evaluate_FrustumFadeAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds, _uint iPlaneMask) const;
	CULLING_FADE_RESULT Evaluate_DistanceFade(const BoundingSphere& WorldBounds, _float fCullDistance, _float fFadeWidth) const;

private:
	CGameInstance_Proxy*	m_pProxy = { nullptr };

	FRUSTUM_VIEW_STATE		m_ViewStates[ETOUI(CULLING_VIEW::END)] = {};

private:
	void	Invalidate_All();
	_float  Compute_SurfaceDistance(const BoundingSphere& WorldBounds) const;
	CULLING_FADE_RESULT Evaluate_DistanceFade(_float fSurfaceDistance, _float fCullDistance, _float fFadeWidth) const;

public:
	static CCulling_Manager* Create();

public:
	virtual void Free() override;
};

NS_END