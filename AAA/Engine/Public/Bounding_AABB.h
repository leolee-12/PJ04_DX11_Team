#pragma once

#include "Bounding.h"

NS_BEGIN(Engine)

class CBounding_AABB final : public CBounding
{
public:
	typedef struct tagBoundingAABBDesc : public CBounding::BOUNDING_DESC
	{
		_float3		vSize;
	}BOUNDING_AABB_DESC;
private:
	CBounding_AABB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBounding_AABB() = default;

public:
	const BoundingBox* Get_Desc() const {
		return m_pDesc;
	}


public:
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pBoundingDesc) override;
	virtual void Update(_fmatrix TransformMatrix) override;
	virtual void Reset_Desc(const CBounding::BOUNDING_DESC* pDesc) override;

public:
	virtual _bool Intersect(COLLIDER eTargetType, CBounding* pBounding) override;

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
	BoundingBox* m_pOriginalDesc = { nullptr };
	BoundingBox* m_pDesc = { nullptr };

private:
	_bool Intersect(CBounding_AABB* pTarget);

	_float3 Compute_Min();
	_float3 Compute_Max();

public:
	static CBounding_AABB* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CBounding::BOUNDING_DESC* pDesc);
	virtual void Free() override;

};

NS_END