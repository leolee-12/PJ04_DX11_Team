#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CBounding abstract : public CBase
{
public:
	typedef struct tagBoundingDesc
	{
		_float3		vCenter;
	}BOUNDING_DESC;


protected:
	CBounding(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBounding() = default;

public:
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc);
	virtual void Update(_fmatrix TransformMatrix) = 0;
	virtual void Reset_Desc(const BOUNDING_DESC* pDesc) = 0;

public:
	virtual _bool Intersect(COLLIDER eTargetType, CBounding* pBounding) = 0;
	void Set_Colliding(_bool b) { m_isColl = b; }

#ifdef _DEBUG
public:
	void Set_DebugColor(const _float4& vColor) { m_vDebugColor = vColor; }
	const _float4& Get_DebugRenderColor() const { return true == m_isColl ? m_vDebugCollidingColor : m_vDebugColor; }
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) = 0;

protected:
	_float4 m_vDebugColor = { 0.f, 1.f, 1.f, 1.f };
	_float4 m_vDebugCollidingColor = { 1.f, 0.f, 0.f, 1.f };
#endif

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_bool					m_isColl = { false };


public:	
	virtual void Free() override;
};

NS_END