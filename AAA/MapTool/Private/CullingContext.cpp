#include "CullingContext.h"

NS_BEGIN(MapTool)

HRESULT CCullingContext::Initialize()
{
	m_WorldFrustum = {};
	m_bHasFrustum = false;

	return S_OK;
}

_bool CCullingContext::Update_FromViewProj(const _float4x4* pView, const _float4x4* pProj)
{
	if (nullptr == pView || nullptr == pProj)
	{
		m_bHasFrustum = false;
		return false;
	}

	return Update_FromViewProj(
		XMLoadFloat4x4(pView),
		XMLoadFloat4x4(pProj));
}

_bool XM_CALLCONV CCullingContext::Update_FromViewProj(_fmatrix ViewMatrix, _fmatrix ProjMatrix)
{
	BoundingFrustum LocalFrustum{};
	BoundingFrustum::CreateFromMatrix(LocalFrustum, ProjMatrix);

	const _matrix ViewInverse = XMMatrixInverse(nullptr, ViewMatrix);
	LocalFrustum.Transform(m_WorldFrustum, ViewInverse);

	m_bHasFrustum = true;
	return true;
}

_bool XM_CALLCONV CCullingContext::IsIn_WorldSpace(_fvector vWorldPos, _float fRange) const
{
	if (!m_bHasFrustum)
		return true;

	BoundingSphere WorldSphere{};
	XMStoreFloat3(&WorldSphere.Center, vWorldPos);
	WorldSphere.Radius = fRange;

	return m_WorldFrustum.Intersects(WorldSphere);
}

_bool CCullingContext::IsIn_WorldSpace_AABB(const BoundingBox& WorldBounds) const
{
	if (!m_bHasFrustum)
		return true;

	return m_WorldFrustum.Intersects(WorldBounds);
}

_bool CCullingContext::Should_CullAABB(_bool bEnableCulling, const BoundingBox& WorldBounds) const
{
	if (!m_bHasFrustum || !bEnableCulling)
		return false;

	return !IsIn_WorldSpace_AABB(WorldBounds);
}

_bool CCullingContext::Is_ValidAABB(const _float3& vMin, const _float3& vMax)
{
	return vMin.x <= vMax.x
		&& vMin.y <= vMax.y
		&& vMin.z <= vMax.z;
}

BoundingBox CCullingContext::Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax)
{
	const _float3 vCenter(
		(vMin.x + vMax.x) * 0.5f,
		(vMin.y + vMax.y) * 0.5f,
		(vMin.z + vMax.z) * 0.5f);

	const _float3 vExtents(
		(vMax.x - vMin.x) * 0.5f,
		(vMax.y - vMin.y) * 0.5f,
		(vMax.z - vMin.z) * 0.5f);

	return BoundingBox(vCenter, vExtents);
}

BoundingBox CCullingContext::Make_DefaultAABB()
{
	return BoundingBox(_float3(0.f, 0.f, 0.f), _float3(1.f, 1.f, 1.f));
}

_bool XM_CALLCONV CCullingContext::Transform_AABB(const BoundingBox& LocalBounds, _fmatrix WorldMatrix, BoundingBox* pOutWorldBounds)
{
	if (nullptr == pOutWorldBounds)
		return false;

	LocalBounds.Transform(*pOutWorldBounds, WorldMatrix);
	return true;
}

BoundingBox CCullingContext::Merge_AABB(const BoundingBox& A, const BoundingBox& B)
{
	_float3 vCornersA[BoundingBox::CORNER_COUNT]{};
	_float3 vCornersB[BoundingBox::CORNER_COUNT]{};
	A.GetCorners(vCornersA);
	B.GetCorners(vCornersB);

	_float3 vMin = vCornersA[0];
	_float3 vMax = vCornersA[0];

	const auto Accumulate = [&vMin, &vMax](const _float3& vPoint)
	{
		vMin.x = (vPoint.x < vMin.x) ? vPoint.x : vMin.x;
		vMin.y = (vPoint.y < vMin.y) ? vPoint.y : vMin.y;
		vMin.z = (vPoint.z < vMin.z) ? vPoint.z : vMin.z;

		vMax.x = (vPoint.x > vMax.x) ? vPoint.x : vMax.x;
		vMax.y = (vPoint.y > vMax.y) ? vPoint.y : vMax.y;
		vMax.z = (vPoint.z > vMax.z) ? vPoint.z : vMax.z;
	};

	for (const _float3& vPoint : vCornersA)
		Accumulate(vPoint);

	for (const _float3& vPoint : vCornersB)
		Accumulate(vPoint);

	return Make_AABB_FromMinMax(vMin, vMax);
}

CCullingContext* CCullingContext::Create()
{
	CCullingContext* pInstance = new CCullingContext();

	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CCullingContext::Free()
{
	__super::Free();
}

NS_END
