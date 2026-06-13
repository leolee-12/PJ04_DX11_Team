#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;

class ENGINE_DLL CFrustum_Manager final : public CBase
{
public:
	struct FRUSTUM_VIEW_STATE
	{
		BoundingFrustum		WorldFrustum = {};
		_bool				bValid = { false };
		_float				fCullMargin = { 0.f };
	};

private:
	CFrustum_Manager();
	virtual ~CFrustum_Manager() = default;

public:
	HRESULT Initialize();
	void	Update();

	_bool	Update_View(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc);
	void	Invalidate_View(CULLING_VIEW eView);
	void	Invalidate_All();

	_bool	Is_Valid(CULLING_VIEW eView) const;

	_bool	XM_CALLCONV IsIn_WorldSpace(CULLING_VIEW eView, _fvector vWorldPos, _float fRange = 0.f) const;
	_bool	IsIn_WorldSpace_AABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const;
	_bool	Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds, _float fRange = 0.f) const;
	_bool   Should_CullByDistance(const BoundingBox& WorldBounds, _float fCullDistance) const;

#ifdef _DEBUG
	void	Reset_Stats();
	const FRUSTUM_CULLING_STATS&	Get_Stats(CULLING_VIEW eView) const;
#endif

private:
	CGameInstance_Proxy*	m_pProxy = { nullptr };
	_bool	m_bEnableFrustumCulling = { true };

	FRUSTUM_VIEW_STATE		m_ViewStates[ETOUI(CULLING_VIEW::END)] = {};

#ifdef _DEBUG
	FRUSTUM_CULLING_STATS   m_Stats[ETOUI(CULLING_VIEW::END)] = {};
#endif

public:
	static CFrustum_Manager* Create();

public:
	virtual void Free() override;
};

NS_END