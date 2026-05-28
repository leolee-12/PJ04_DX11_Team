#pragma once

#include "Bounding.h"

NS_BEGIN(Engine)

class CBounding_OBB final : public CBounding
{
public:
	typedef struct tagBoundingOBBDesc : public CBounding::BOUNDING_DESC
	{
		_float3		vSize;
		_float3		vRadians;
	}BOUNDING_OBB_DESC;

	typedef struct tagOBBDesc
	{
		_float3		vCenter;
		_float3		vCenterDir[3];
		_float3		vAlignDir[3];
	}OBB_DESC;
private:
	CBounding_OBB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBounding_OBB() = default;
public:
	const BoundingOrientedBox* Get_Desc() const {
		return m_pDesc;
	}
public:
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc) override;
	virtual void Update(_fmatrix TransformMatrix) override;

public:
	virtual _bool Intersect(COLLIDER eTargetType, CBounding* pBounding) override;
#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
	BoundingOrientedBox* m_pOriginalDesc = { nullptr };
	BoundingOrientedBox* m_pDesc = { nullptr };

private:
	_bool Intersect(CBounding_OBB* pTarget);
	OBB_DESC Compute_OBB();


public:
	static CBounding_OBB* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CBounding::BOUNDING_DESC* pDesc);
	virtual void Free() override;

};

NS_END