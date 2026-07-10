#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "Bounding_Capsule.h"
#include "DebugDraw.h"
#include "DebugCapsule.h"

CBounding_Sphere::CBounding_Sphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBounding { pDevice, pContext }
{
}

HRESULT CBounding_Sphere::Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc)
{
    auto    pDesc = static_cast<const CBounding_Sphere::BOUNDING_SPHERE_DESC*>(pBoundingDesc);

    m_pOriginalDesc = new BoundingSphere(pDesc->vCenter, pDesc->fRadius);
    m_pDesc = new BoundingSphere(*m_pOriginalDesc);

	return S_OK;
}

void CBounding_Sphere::Update(_fmatrix TransformMatrix)
{   
    m_pOriginalDesc->Transform(*m_pDesc, TransformMatrix);
}

void CBounding_Sphere::Reset_Desc(const CBounding::BOUNDING_DESC* pDesc)
{
    auto pSphereDesc = static_cast<const BOUNDING_SPHERE_DESC*>(pDesc);

    m_pOriginalDesc->Center = pSphereDesc->vCenter;
    m_pOriginalDesc->Radius = pSphereDesc->fRadius;
    *m_pDesc = *m_pOriginalDesc;
}

_bool CBounding_Sphere::Intersect(COLLIDER eTargetType, CBounding* pBounding)
{
    m_isColl = false;

    switch (eTargetType)
    {
    case COLLIDER::AABB:
        m_isColl = m_pDesc->Intersects(*dynamic_cast<CBounding_AABB*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::OBB:
        m_isColl = m_pDesc->Intersects(*dynamic_cast<CBounding_OBB*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::SPHERE:
        m_isColl = m_pDesc->Intersects(*dynamic_cast<CBounding_Sphere*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::CAPSULE:
        m_isColl = dynamic_cast<CBounding_Capsule*>(pBounding)->Intersects_Sphere(m_pDesc);
        break;
    }

    return m_isColl;
}

#ifdef _DEBUG

HRESULT CBounding_Sphere::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
    const _float4& vColor = Get_DebugRenderColor();
    Debug_DrawSphere(pBatch, XMLoadFloat3(&m_pDesc->Center), m_pDesc->Radius, XMLoadFloat4(&vColor));

    return S_OK;
}

#endif

CBounding_Sphere* CBounding_Sphere::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CBounding::BOUNDING_DESC* pDesc)
{
    CBounding_Sphere* pInstance = new CBounding_Sphere(pDevice, pContext);

    if (FAILED(pInstance->Initialize(pDesc)))
    {
        MSG_BOX("Failed to Created : CBounding_Sphere");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBounding_Sphere::Free()
{
	__super::Free();

    Safe_Delete(m_pDesc);
    Safe_Delete(m_pOriginalDesc);
}
