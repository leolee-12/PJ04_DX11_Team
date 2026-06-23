#pragma once

#include "Component.h"

NS_BEGIN(physx)
class PxTriangleMesh;
NS_END

NS_BEGIN(Engine)

class CMesh;
class CMaterial;
class CMaterialEx;
class CBone;
class CAnimation;

class ENGINE_DLL CModel final : public CComponent
{
private:
	using PickableFilter = function<bool(const string&)>;

private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	_uint Get_NumAnimations() const { return static_cast<_uint>(m_Animations.size()); }
	_uint Get_NumBones() const { return static_cast<_uint>(m_Bones.size()); }
	const string& Get_BoneName(_uint iIndex) const;
	_int Get_BoneParentIndex(_uint iIndex) const;
	_int Get_RootBoneIndex() const;
	void Get_AnimChannelBoneIndices(_uint iAnimIndex, vector<_uint>& Out);

	size_t Get_NumMeshes() const { return m_iNumMeshes; }
	_uint Get_MaxAnimationIndex() const { return m_Animations.empty() ? 0u : static_cast<_uint>(m_Animations.size()) - 1u; }
	_int Get_BoneIndex(const string& strBoneName);

	const _float4x4* Get_BoneMatrixPtr(const string& strBoneName) const;

	const string& Get_AnimationName(_uint iIndex) const;

	const string& Get_MeshName(_uint iIndex) const;

	void Get_MeshAABB(_uint iIndex, _float3* pOutMin, _float3* pOutMax) const;
	void Get_ModelAABB(_float3* pOutMin, _float3* pOutMax) const;

	_float Get_CurrentAnimProgress() const;

public:
	_bool Pick_Mesh(_uint iMeshIndex, _fvector vOrigin, _fvector vDir, _fmatrix WorldMatrix, _float3* pOutHit, float* pOutDist = nullptr);
	physx::PxTriangleMesh* Get_CollisionMesh() const { return m_pCollisionMesh; }

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix, PickableFilter fcFillter = nullptr);
	virtual HRESULT Initialize_Prototype_WithTextureHub(MODEL eType, const _char* pModelFilePath,
		_fmatrix PreTransformMatrix, PickableFilter fcFillter = nullptr, _bool bCookCollisionMesh = false);
	virtual HRESULT Initialize(void* pArg);

public:
	void			Set_AnimationIndex(_uint iIndex, _bool isLoop = false, _bool isRestart = false, _float fBlendDuration = 0.2f);
	_int			Get_AnimationIndex(const string& strName) const;
	const string&	Get_CurrentAnimName() const;
	void			Seek_Animation(_float fProgress);

	_bool			Update_Base(_float fTimeDelta, _float fSpeed = 1.0f);
	_bool			Play_Animation(_float fTimeDelta, _float fSpeed = 1.0f);
	_bool			Play_Animation(_float fTimeDelta, _float fMaskTimeDelta, _int iMaskIndex, _float& fMaskLocalTime, vector<_uint>& MaskCursors,
									_float fSpeed = 1.0f, _float fMaskSpeed = 1.0f, _bool bLoop = true, _float fMaskWeight = 1.f, _bool* pbOverlayFinished = nullptr);

	_bool			Apply_Mask(_int iIndex, _float& fLocalTime, vector<_uint>& Cursors, _float fTimeDelta, _float fSpeed = 1.0f, _bool bLoop = true, _float fMaskWeight = 1.f);
	void			Update_Combined();

	HRESULT			Render(_uint iMeshIndex);

	void			Build_MaskBones(const vector<_string>& Roots);

	void			Clear_MaskBones() { m_MaskBones.clear(); }

	const _wstring& Get_ModelPath() { return m_strModelPath; }

public:
	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, MTEX_TYPE eType, _uint iIndex);
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);

public:
	const MESH_LAYER_IDX& Get_MeshLayer(_uint iMesh) const;
	void  Set_MeshLayer(_uint iMesh, const MESH_LAYER_IDX& v);
	_uint Get_MeshTextureCount(_uint iMesh, MTEX_TYPE eType) const;
	HRESULT Save_MeshLayers() const;

public:	// Instance
	HRESULT Render_Instanced(_uint iMeshIndex, ID3D11Buffer* pInstanceBuffer, _uint iInstanceStride, _uint iInstanceCount);

private:
	MODEL						m_eType = { MODEL::END };
	_wstring					m_strModelPath = {};
	PickableFilter              m_PickableFilter = { nullptr };

private:
	size_t						m_iNumMeshes = {};
	vector<CMesh*>				m_Meshes;

	size_t						m_iNumMaterials = {};
	vector<CMaterial*>			m_Materials;
	_bool                       m_bUseMaterialEx = { false };
	vector<CMaterialEx*>        m_MaterialsEx;
	_bool						m_bCookCollisionMesh = { false };

	_float4x4					m_PreTransformMatrix = {};	
	vector<CBone*>				m_Bones;

	_uint						m_iNumAnimations = {};
	vector<CAnimation*>			m_Animations;

	_uint						m_iCurrentAnimationIndex = { (_uint)-1 };
	_bool						m_isAnimLoop = { false };

	_uint						m_iBlendTargetAnimIndex = { (_uint)-1 };
	_float						m_fBlendDuration = { };
	_float						m_fBlendElapsed = { };
	_bool						m_isBlending = { false };

	physx::PxTriangleMesh*		m_pCollisionMesh = { nullptr };

	// юс╫ц
	vector<MESH_LAYER_IDX>		m_MeshLayers;
	string						m_strMeshLayerPath;
	
	vector<_uint>				m_MaskBones;


private:
	HRESULT Ready_Meshes(const vector<MESH_DATA>& meshes, _fmatrix PreTransformMatrix);
	HRESULT Ready_Materials(const vector<MATERIAL_DATA>& materials, const _char* pModelFilePath);
	HRESULT Ready_MaterialsEx(const vector<MATERIAL_DATA>& materials);
	HRESULT Ready_Bones(const vector<BONE_DATA>& bones);
	HRESULT Ready_Animations(const vector<ANIMATION_DATA>& animations);

	HRESULT Ready_NonAnimEx(const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	HRESULT Ready_NonAnim(const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	HRESULT Ready_Anim(const _char* pModelFilePath, _fmatrix PreTransformMatrix);

private:
	void Load_MeshLayers(const _char* pModelFilePath);
	HRESULT Cook_CollisionMesh(const vector<MESH_DATA>& meshes, _fmatrix PreTransformMatrix);

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity(), PickableFilter fcFillter = nullptr);
	static CModel* Create_WithTextureHub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath,
		_fmatrix PreTransformMatrix = XMMatrixIdentity(), PickableFilter fcFillter = nullptr, _bool bCookCollisionMesh = false);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END