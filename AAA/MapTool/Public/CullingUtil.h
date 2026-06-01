#pragma once

#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(MapTool)

class CCullingUtil final : public CBase
{
private:
	CCullingUtil() = default;
	virtual ~CCullingUtil() = default;

public:
	_bool Build_WorldFrustum(CGameInstance_Proxy* pGI_Proxy, PROJ_TYPE eProjType, BoundingFrustum* pOutFrustum) const;
	_bool Build_WorldFrustum(const _float4x4* pView, const _float4x4* pProj, BoundingFrustum* pOutFrustum) const;
	_bool Is_AABBVisible(const BoundingFrustum& WorldFrustum, const BoundingBox& WorldBounds) const;
	_bool Should_CullAABB(_bool bHasFrustum, _bool bEnableCulling, const BoundingFrustum& WorldFrustum, const BoundingBox& WorldBounds) const;

public:
	static CCullingUtil* Create();

protected:
	virtual void Free() override;
};

NS_END
