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

public:
	virtual _bool Intersect(COLLIDER eTargetType, CBounding* pBounding) = 0;
	void Set_Colliding(_bool b) { m_isColl = b; }

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) = 0;
#endif

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_bool					m_isColl = { false };


public:	
	virtual void Free() override;
};

NS_END