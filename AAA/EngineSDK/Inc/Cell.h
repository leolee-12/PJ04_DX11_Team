#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CVIBuffer_Cell;

class CCell final : public CBase
{

private:
	CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCell() = default;

public:
	_fvector Get_Point(NAVI_POINT ePoint) {
		return XMLoadFloat3(&m_vPoints[ETOUI(ePoint)]);
	}

public:
	void Set_Neighbor(NAVI_LINE eLine, _uint iNeighborIndex) {
		m_iNeighbors[ETOUI(eLine)] = iNeighborIndex;
	}

	_uint Get_Neighbor(NAVI_LINE eLine) const {
		return m_iNeighbors[ETOUI(eLine)];
	}

	_fvector  Get_Centroid() const {
		return XMLoadFloat3(&m_vCentroid);
	}

	_uint Get_Index() const { return m_iIndex; }

	_float3 Get_Normal(NAVI_LINE eLine) const { return m_vNormals[ETOUI(eLine)]; }

public:
	HRESULT Initialize(const _float3* pPoints, _uint iIndex);
	_bool   Query_isIn(_fvector vResultPos, _uint* pNeighborIndex) const;
	_float Compute_Height(_fvector vTargetPos);

#ifdef _DEBUG
public:
	HRESULT Render();
#endif
private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_float3					m_vPoints[ETOUI(NAVI_POINT::END)] = {};
	_float3					m_vNormals[ETOUI(NAVI_LINE::END)] = {};
	_uint					m_iIndex = {};
	_uint					m_iNeighbors[ETOUI(NAVI_LINE::END)] = { (_uint)-1, (_uint)-1, (_uint)-1 };
	_float4					m_vPlane = {};

	_float3					m_vCentroid = {};

#ifdef _DEBUG
private:
	CVIBuffer_Cell*	m_pVIBuffer = { nullptr };
#endif

public:
	static CCell* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints, _uint iIndex);
	virtual void Free() override;
};

NS_END