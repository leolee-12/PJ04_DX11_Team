#include "Cell.h"
#ifdef _DEBUG
#include "VIBuffer_Cell.h"
#endif

CCell::CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCell::Initialize(const _float3* pPoints, _uint iIndex)
{
	m_iIndex = iIndex;

	memcpy(m_vPoints, pPoints, sizeof(_float3) * ETOUI(NAVI_POINT::END));

	XMStoreFloat4(&m_vPlane,
		XMPlaneFromPoints(XMLoadFloat3(&m_vPoints[ETOUI(NAVI_POINT::A)]), XMLoadFloat3(&m_vPoints[ETOUI(NAVI_POINT::B)]), XMLoadFloat3(&m_vPoints[ETOUI(NAVI_POINT::C)])));

	m_vNormals[ETOUI(NAVI_LINE::AB)] = _float3((m_vPoints[ETOUI(NAVI_POINT::B)].z - m_vPoints[ETOUI(NAVI_POINT::A)].z) * -1.f, 0.f, m_vPoints[ETOUI(NAVI_POINT::B)].x - m_vPoints[ETOUI(NAVI_POINT::A)].x);
	m_vNormals[ETOUI(NAVI_LINE::BC)] = _float3((m_vPoints[ETOUI(NAVI_POINT::C)].z - m_vPoints[ETOUI(NAVI_POINT::B)].z) * -1.f, 0.f, m_vPoints[ETOUI(NAVI_POINT::C)].x - m_vPoints[ETOUI(NAVI_POINT::B)].x);
	m_vNormals[ETOUI(NAVI_LINE::CA)] = _float3((m_vPoints[ETOUI(NAVI_POINT::A)].z - m_vPoints[ETOUI(NAVI_POINT::C)].z) * -1.f, 0.f, m_vPoints[ETOUI(NAVI_POINT::A)].x - m_vPoints[ETOUI(NAVI_POINT::C)].x);

	for (size_t i = 0; i < ETOUI(NAVI_LINE::END); i++)
		XMStoreFloat3(&m_vNormals[i], XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])));

	XMStoreFloat3(&m_vCentroid,
		(XMLoadFloat3(&m_vPoints[ETOUI(NAVI_POINT::A)]) + XMLoadFloat3(&m_vPoints[ETOUI(NAVI_POINT::B)]) + XMLoadFloat3(&m_vPoints[ETOUI(NAVI_POINT::C)])) / 3.f);

#ifdef _DEBUG
	/*m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_vPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;*/
#endif
	
	return S_OK;
}

_bool CCell::Query_isIn(_fvector vResultPos, _uint* pNeighborIndex) const
{
	for (size_t i = 0; i < ETOUI(NAVI_LINE::END); i++)
	{
		_vector		vDir = XMVector3Normalize(vResultPos - XMLoadFloat3(&m_vPoints[i]));
		_vector		vNormal = XMLoadFloat3(&m_vNormals[i]);

		if (0 < XMVectorGetX(XMVector3Dot(vDir, vNormal)))
		{
			*pNeighborIndex = m_iNeighbors[i];
			return false;
		}
	}

	return true;
}

_float CCell::Compute_Height(_fvector vTargetPos)
{
	/* ax + by + cz + d = 0 */
	/* y = (-ax -cz - d) / b */
	if (fabsf(m_vPlane.y) < FLT_EPSILON)
		return XMVectorGetY(vTargetPos);

	return (-m_vPlane.x * XMVectorGetX(vTargetPos) - m_vPlane.z * XMVectorGetZ(vTargetPos) - m_vPlane.w) / m_vPlane.y;
}

#ifdef _DEBUG
HRESULT CCell::Render()
{
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	return m_pVIBuffer->Render();
}
#endif

CCell* CCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints, _uint iIndex)
{
	CCell* pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pPoints, iIndex)))
	{
		MSG_BOX("Failed to Created : CCell");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCell::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);
#endif

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
