#include "Culling_Context.h"

_bool CCulling_Context::Intersects(const BoundingBox& WorldBounds) const
{
	if (!m_bHasFrustum)
		return true;

	BoundingBox ExpandedBounds = WorldBounds;
	if (m_fCullMargin > 0.f)
		CCulling_Util::Expand_AABB(&ExpandedBounds, m_fCullMargin);

	return m_WorldFrustum.Intersects(ExpandedBounds);
}

_bool CCulling_Context::Intersects(const BoundingSphere& WorldBounds) const
{
	if (!m_bHasFrustum)
		return true;

	BoundingSphere ExpandedBounds = WorldBounds;
	if (m_fCullMargin > 0.f)
		ExpandedBounds.Radius += m_fCullMargin;

	return m_WorldFrustum.Intersects(ExpandedBounds);
}

_bool CCulling_Context::Should_CullAABB(_bool bEnableCulling, const BoundingBox& WorldBounds) const
{
	if (!bEnableCulling)
		return false;

	return !Intersects(WorldBounds);
}

_bool CCulling_Context::Should_CullSphere(_bool bEnableCulling, const BoundingSphere& WorldBounds)
const
{
	if (!bEnableCulling)
		return false;

	return !Intersects(WorldBounds);
}

const _tchar* CCulling_Context::Get_DebugName() const
{
	if (m_strDebugName.empty())
		return TEXT("UnnamedCullingContext");

	return m_strDebugName.c_str();
}

_bool CCulling_Context::Try_BuildWorldFrustum(const CULLING_CONTEXT_DESC& tDesc, BoundingFrustum*
	pOutWorldFrustum)
{
	if (nullptr == pOutWorldFrustum)
		return false;

	if (nullptr == tDesc.pView || nullptr == tDesc.pProj)
		return false;

	BoundingFrustum LocalFrustum{};
	BoundingFrustum::CreateFromMatrix(LocalFrustum, XMLoadFloat4x4(tDesc.pProj));

	_matrix ViewInverse = XMMatrixIdentity();
	if (nullptr != tDesc.pInvView)
		ViewInverse = XMLoadFloat4x4(tDesc.pInvView);
	else
		ViewInverse = XMMatrixInverse(nullptr, XMLoadFloat4x4(tDesc.pView));

	LocalFrustum.Transform(*pOutWorldFrustum, ViewInverse);
	return true;
}

CCulling_Context CCulling_Context::Create(const CULLING_CONTEXT_DESC& tDesc)
{
	CCulling_Context tContext{};

	tContext.m_fNearZ = tDesc.fNearZ;
	tContext.m_fFarZ = tDesc.fFarZ;
	tContext.m_fCullMargin = tDesc.fCullMargin;
	tContext.m_eViewType = tDesc.eViewType;

	if (tContext.m_fCullMargin < 0.f)
		tContext.m_fCullMargin = 0.f;

	if (nullptr != tDesc.pEyePos)
		tContext.m_vEyePos = *tDesc.pEyePos;

	if (nullptr != tDesc.pDebugName && 0 != tDesc.pDebugName[0])
		tContext.m_strDebugName = tDesc.pDebugName;
	else
		tContext.m_strDebugName = TEXT("UnnamedCullingContext");

	tContext.m_bHasFrustum = Try_BuildWorldFrustum(tDesc, &tContext.m_WorldFrustum);
	return tContext;
}

void CCulling_Context::Free()
{
}