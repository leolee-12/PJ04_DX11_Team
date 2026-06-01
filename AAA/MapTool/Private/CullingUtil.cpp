#include "CullingUtil.h"

#include "GameInstance_Proxy.h"

NS_BEGIN(MapTool)

_bool CCullingUtil::Build_WorldFrustum(CGameInstance_Proxy* pGI_Proxy, PROJ_TYPE eProjType, BoundingFrustum* pOutFrustum) const
{
	if (nullptr == pGI_Proxy)
		return false;

	return Build_WorldFrustum(
		pGI_Proxy->Get_Matrix(D3DTS::VIEW, eProjType),
		pGI_Proxy->Get_Matrix(D3DTS::PROJ, eProjType),
		pOutFrustum);
}

_bool CCullingUtil::Build_WorldFrustum(const _float4x4* pView, const _float4x4* pProj, BoundingFrustum* pOutFrustum) const
{
	if (nullptr == pView || nullptr == pProj || nullptr == pOutFrustum)
		return false;

	BoundingFrustum LocalFrustum{};
	BoundingFrustum::CreateFromMatrix(LocalFrustum, XMLoadFloat4x4(pProj));
	LocalFrustum.Transform(*pOutFrustum, XMMatrixInverse(nullptr, XMLoadFloat4x4(pView)));

	return true;
}

_bool CCullingUtil::Is_AABBVisible(const BoundingFrustum& WorldFrustum, const BoundingBox& WorldBounds) const
{
	return WorldFrustum.Intersects(WorldBounds);
}

_bool CCullingUtil::Should_CullAABB(_bool bHasFrustum, _bool bEnableCulling, const BoundingFrustum& WorldFrustum, const BoundingBox& WorldBounds) const
{
	if (!bHasFrustum || !bEnableCulling)
		return false;

	return !Is_AABBVisible(WorldFrustum, WorldBounds);
}

CCullingUtil* CCullingUtil::Create()
{
	return new CCullingUtil();
}

void CCullingUtil::Free()
{
	__super::Free();
}

NS_END
