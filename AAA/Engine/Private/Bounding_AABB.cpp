#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "Bounding_Capsule.h"
#include "DebugDraw.h"

CBounding_AABB::CBounding_AABB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBounding { pDevice, pContext }
{
}

HRESULT CBounding_AABB::Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc)
{
    auto    pDesc = static_cast<const CBounding_AABB::BOUNDING_AABB_DESC*>(pBoundingDesc);

    m_pOriginalDesc = new BoundingBox(pDesc->vCenter, _float3(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f));
    m_pDesc = new BoundingBox(*m_pOriginalDesc);

	return S_OK;
}

void CBounding_AABB::Update(_fmatrix TransformMatrix)
{
    _matrix   Matrix = TransformMatrix;
    
    Matrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(Matrix.r[0]);
    Matrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(Matrix.r[1]);
    Matrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(Matrix.r[2]);

    m_pOriginalDesc->Transform(*m_pDesc, Matrix);
}

_bool CBounding_AABB::Intersect(COLLIDER eTargetType, CBounding* pBounding)
{
    m_isColl = false;

    switch (eTargetType)
    {
    case COLLIDER::AABB:
        m_isColl = m_pDesc->Intersects(*static_cast<CBounding_AABB*>(pBounding)->Get_Desc());
        //m_isColl = Intersect(dynamic_cast<CBounding_AABB*>(pBounding));
        break;

    case COLLIDER::OBB:
        m_isColl = m_pDesc->Intersects(*static_cast<CBounding_OBB*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::SPHERE:
        m_isColl = m_pDesc->Intersects(*static_cast<CBounding_Sphere*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::CAPSULE:
        m_isColl = dynamic_cast<CBounding_Capsule*>(pBounding)->Intersects_AABB(m_pDesc);
        break;
    }

    return m_isColl;
}

#ifdef _DEBUG

HRESULT CBounding_AABB::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
    
    DX::Draw(pBatch, *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 1.f, 1.f));

    return S_OK;
}

#endif

_bool CBounding_AABB::Intersect(CBounding_AABB* pTarget)
{
    _float3     vSourMin = Compute_Min();
    _float3     vSourMax = Compute_Max();
    _float3     vDestMin = pTarget->Compute_Min();
    _float3     vDestMax = pTarget->Compute_Max();

    /* 너비비교. */
    if (max(vSourMin.x, vDestMin.x) > min(vSourMax.x, vDestMax.x))
        return false;
    if (max(vSourMin.y, vDestMin.y) > min(vSourMax.y, vDestMax.y))
        return false;
    if (max(vSourMin.z, vDestMin.z) > min(vSourMax.z, vDestMax.z))
        return false;


    return true;
}

_float3 CBounding_AABB::Compute_Min()
{
    return _float3(m_pDesc->Center.x - m_pDesc->Extents.x, 
        m_pDesc->Center.y - m_pDesc->Extents.y, 
        m_pDesc->Center.z - m_pDesc->Extents.z);
}

_float3 CBounding_AABB::Compute_Max()
{
    return _float3(m_pDesc->Center.x + m_pDesc->Extents.x,
        m_pDesc->Center.y + m_pDesc->Extents.y,
        m_pDesc->Center.z + m_pDesc->Extents.z);
}

CBounding_AABB* CBounding_AABB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CBounding::BOUNDING_DESC* pDesc)
{
    CBounding_AABB* pInstance = new CBounding_AABB(pDevice, pContext);

    if (FAILED(pInstance->Initialize(pDesc)))
    {
        MSG_BOX("Failed to Created : CBounding_AABB");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBounding_AABB::Free()
{
	__super::Free();

    Safe_Delete(m_pDesc);
    Safe_Delete(m_pOriginalDesc);
}
