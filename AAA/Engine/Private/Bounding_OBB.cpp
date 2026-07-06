#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"
#include "Bounding_Capsule.h"
#include "DebugDraw.h"

CBounding_OBB::CBounding_OBB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBounding { pDevice, pContext }
{
}

HRESULT CBounding_OBB::Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc)
{
    auto    pDesc = static_cast<const CBounding_OBB::BOUNDING_OBB_DESC*>(pBoundingDesc);
    
    _float4     vQuaternion = {};
    XMStoreFloat4(&vQuaternion, XMQuaternionRotationRollPitchYaw(pDesc->vRadians.x, pDesc->vRadians.y, pDesc->vRadians.z));

    m_pOriginalDesc = new BoundingOrientedBox(pDesc->vCenter, _float3(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f), vQuaternion);
    m_pDesc = new BoundingOrientedBox(*m_pOriginalDesc);

	return S_OK;
}

void CBounding_OBB::Update(_fmatrix TransformMatrix)
{
  
    m_pOriginalDesc->Transform(*m_pDesc, TransformMatrix);
}

void CBounding_OBB::Reset_Desc(const CBounding::BOUNDING_DESC* pDesc)
{
    auto pOBBDesc = static_cast<const BOUNDING_OBB_DESC*>(pDesc);

    _float4         vQuaternion = {};
    XMStoreFloat4(&vQuaternion, XMQuaternionRotationRollPitchYaw(pOBBDesc->vRadians.x, pOBBDesc->vRadians.y, pOBBDesc->vRadians.z));

    m_pOriginalDesc->Center = pOBBDesc->vCenter;
    m_pOriginalDesc->Extents = _float3(pOBBDesc->vSize.x * 0.5f, pOBBDesc->vSize.y * 0.5f, pOBBDesc->vSize.z * 0.5f);
    m_pOriginalDesc->Orientation = vQuaternion;
    *m_pDesc = *m_pOriginalDesc;
}

_bool CBounding_OBB::Intersect(COLLIDER eTargetType, CBounding* pBounding)
{
    m_isColl = false;

    switch (eTargetType)
    {
    case COLLIDER::AABB:
        m_isColl = m_pDesc->Intersects(*dynamic_cast<CBounding_AABB*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::OBB:
        m_isColl = m_pDesc->Intersects(*dynamic_cast<CBounding_OBB*>(pBounding)->Get_Desc());
        //m_isColl = Intersect(dynamic_cast<CBounding_OBB*>(pBounding));
        break;

    case COLLIDER::SPHERE:
        m_isColl = m_pDesc->Intersects(*dynamic_cast<CBounding_Sphere*>(pBounding)->Get_Desc());
        break;

    case COLLIDER::CAPSULE:
        m_isColl = dynamic_cast<CBounding_Capsule*>(pBounding)->Intersects_OBB(m_pDesc); 
        break;
    }

    return m_isColl;
}

#ifdef _DEBUG

HRESULT CBounding_OBB::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
    
    DX::Draw(pBatch, *m_pDesc, true == m_isColl ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 1.f, 1.f));

    return S_OK;
}

#endif

_bool CBounding_OBB::Intersect(CBounding_OBB* pTarget)
{
    OBB_DESC     OBBDesc[2] = {
        Compute_OBB(), 
        pTarget->Compute_OBB()
    };

    _float      fDistance[3] = {};

    for (size_t i = 0; i < 2; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            fDistance[0] = fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenter) - XMLoadFloat3(&OBBDesc[0].vCenter),
                XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

            fDistance[1] =
                fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vCenterDir[0]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
                fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vCenterDir[1]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
                fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vCenterDir[2]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

            fDistance[2] =
                fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenterDir[0]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
                fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenterDir[1]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
                fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenterDir[2]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

            if (fDistance[0] > fDistance[1] + fDistance[2])
                return false;

        }
    }

    return true;
}

CBounding_OBB::OBB_DESC CBounding_OBB::Compute_OBB()
{
    OBB_DESC        OBBDesc{};

    OBBDesc.vCenter = m_pDesc->Center;

    _float3     vPoints[8] = {};
    m_pDesc->GetCorners(vPoints);

    XMStoreFloat3(&OBBDesc.vCenterDir[0],
        XMLoadFloat3(&vPoints[5]) - XMLoadFloat3(&vPoints[4]));
    XMStoreFloat3(&OBBDesc.vCenterDir[1],
        XMLoadFloat3(&vPoints[7]) - XMLoadFloat3(&vPoints[4]));
    XMStoreFloat3(&OBBDesc.vCenterDir[2],
        XMLoadFloat3(&vPoints[0]) - XMLoadFloat3(&vPoints[4]));

    for (size_t i = 0; i < 3; i++)
    {
        XMStoreFloat3(&OBBDesc.vCenterDir[i], XMLoadFloat3(&OBBDesc.vCenterDir[i]) * 0.5f);
        XMStoreFloat3(&OBBDesc.vAlignDir[i], XMVector3Normalize(XMLoadFloat3(&OBBDesc.vCenterDir[i])));
    }

    return OBBDesc;
}

CBounding_OBB* CBounding_OBB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CBounding::BOUNDING_DESC* pDesc)
{
    CBounding_OBB* pInstance = new CBounding_OBB(pDevice, pContext);

    if (FAILED(pInstance->Initialize(pDesc)))
    {
        MSG_BOX("Failed to Created : CBounding_OBB");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBounding_OBB::Free()
{
	__super::Free();

    Safe_Delete(m_pDesc);
    Safe_Delete(m_pOriginalDesc);
}
