#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class CModel;
class CShader;
class CBone;

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& Prototype);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}
	const string& Get_Name() const { return m_strName; }

	const _float3& Get_AABBMin() const { return m_vAABBMin; }
	const _float3& Get_AABBMax() const { return m_vAABBMax; }

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, CModel* pOwner, const MESH_DATA& data, _fmatrix PreTransformMatrix, _bool bPickable = false);
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, const vector<CBone*>& Bones);

	_bool Ray_AABB(_fvector vOrigin, _fvector vDir) const;
	_bool Pick(_fvector vOrigin, _fvector vDir, _float3* pOutHit, _float* pOutDist = nullptr) const;

private:
	string				m_strName = {};	
	_uint				m_iMaterialIndex = {  };
	_uint				m_iNumBones = {  };
	vector<_uint>		m_BoneIndices;
	_float4x4			m_BoneMatrices[g_iNumMeshBones] = {};
	vector<_float4x4>	m_BoneOffsetMatrices;

	_bool				m_bPickable = false;
	vector<_float3>		m_PickingPositions;
	vector<_uint>		m_PickingIndices;
	_float3				m_vAABBMin = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	_float3				m_vAABBMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

private:
	HRESULT Ready_NonAnim(const MESH_DATA& data, _fmatrix PreTransformMatrix);
	HRESULT Ready_Map(const MESH_DATA& data, _fmatrix PreTransformMatrix);
	HRESULT Ready_Anim(CModel* pOwner, const MESH_DATA& data);
public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, CModel* pOwner, const MESH_DATA& data, _fmatrix PreTransformMatrix, _bool bPickable = false);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END