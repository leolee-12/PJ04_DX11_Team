#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialEx.h"
#include "DataLoader.h"
#include "Bone.h"
#include "Animation.h"
#include "Animator.h"
#include "MeshLayer_Utils.h"
#include <fstream>

#include "GameInstance.h"
#include "PhysX_Manager.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
	: CComponent(Prototype)
	, m_eType{ Prototype.m_eType }
	, m_iNumMeshes{ Prototype.m_iNumMeshes }
	, m_Meshes{ Prototype.m_Meshes }
	, m_iNumMaterials{ Prototype.m_iNumMaterials }
	, m_Materials{ Prototype.m_Materials }
	, m_bUseMaterialEx{ Prototype.m_bUseMaterialEx }
	, m_MaterialsEx{ Prototype.m_MaterialsEx }
	, m_bCookCollisionMesh{ Prototype.m_bCookCollisionMesh }
	, m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
	, m_iNumAnimations{ Prototype.m_iNumAnimations }
	, m_MeshLayers{ Prototype.m_MeshLayers }
	, m_strMeshLayerPath{ Prototype.m_strMeshLayerPath }
	, m_strModelPath {Prototype.m_strModelPath}
{
	for (auto& pPrototypeAnimation : Prototype.m_Animations)
		m_Animations.push_back(pPrototypeAnimation->Clone());

	for (auto& pPrototypeBone : Prototype.m_Bones)
		m_Bones.push_back(pPrototypeBone->Clone());

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pMaterialEx : m_MaterialsEx)
		Safe_AddRef(pMaterialEx);

	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	if (Prototype.m_pCollisionMesh) {
		m_pCollisionMesh = Prototype.m_pCollisionMesh;
		m_pCollisionMesh->acquireReference();   // 메시/머티리얼 AddRef와 동일 의미
	}
}

const string& CModel::Get_BoneName(_uint iIndex) const
{
	static const string s_Empty;
	return (iIndex < m_Bones.size()) ? m_Bones[iIndex]->Get_Name() : s_Empty;
}

_int CModel::Get_BoneParentIndex(_uint iIndex) const
{
	return (iIndex < m_Bones.size()) ? m_Bones[iIndex]->Get_ParentIndex() : -1;
}

_int CModel::Get_RootBoneIndex() const
{
	for (_uint i = 0; i < static_cast<_uint>(m_Bones.size()); i++)
		if (m_Bones[i]->Get_ParentIndex() < 0)
			return static_cast<_int>(i);

	return m_Bones.empty() ? -1 : 0;
}

void CModel::Get_AnimChannelBoneIndices(_uint iAnimIndex, vector<_uint>& Out)
{
	Out.clear();
	if (iAnimIndex < m_Animations.size())
		m_Animations[iAnimIndex]->Get_ChannelBoneIndices(Out);
}

_int CModel::Get_BoneIndex(const string& strBoneName)
{
	_int        iIndex = { -1 };

	auto    iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
		{
			++iIndex;
			return pBone->Compare_Name(strBoneName);
		});

	if (iter == m_Bones.end())
		return -1;

	return iIndex;
}

const _float4x4* CModel::Get_BoneMatrixPtr(const string& strBoneName) const
{
	auto    iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
		{
			return pBone->Compare_Name(strBoneName);
		});

	if (iter == m_Bones.end())
		return nullptr;

	return (*iter)->Get_CombinedTransformationMatrixPtr();
}

const string& CModel::Get_AnimationName(_uint iIndex) const
{
	return m_Animations[iIndex]->Get_AnimationName();
}

const string& CModel::Get_MeshName(_uint iIndex) const
{
	return m_Meshes[iIndex]->Get_Name();
}

void CModel::Get_MeshAABB(_uint iIndex, _float3* pOutMin, _float3* pOutMax) const
{
	if (iIndex >= m_iNumMeshes || !pOutMin || !pOutMax) return;

	*pOutMin = m_Meshes[iIndex]->Get_AABBMin();
	*pOutMax = m_Meshes[iIndex]->Get_AABBMax();
}

void CModel::Get_ModelAABB(_float3* pOutMin, _float3* pOutMax) const
{
	_float3 mn = { FLT_MAX,  FLT_MAX,  FLT_MAX };
	_float3 mx = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (auto* pMesh : m_Meshes)
	{
		const _float3& a = pMesh->Get_AABBMin();
		const _float3& b = pMesh->Get_AABBMax();
		mn.x = min(mn.x, a.x); mn.y = min(mn.y, a.y); mn.z = min(mn.z, a.z);
		mx.x = max(mx.x, b.x); mx.y = max(mx.y, b.y); mx.z = max(mx.z, b.z);
	}

	*pOutMin = mn;
	*pOutMax = mx;
}

_float CModel::Get_CurrentAnimProgress() const
{
	_uint idx = m_isBlending ? m_iBlendTargetAnimIndex : m_iCurrentAnimationIndex;
	if (idx == (_uint)-1 || idx >= m_Animations.size())
		return 0.f;
	return m_Animations[idx]->Get_Progress();
}

_bool CModel::Pick_Mesh(_uint iMeshIndex, _fvector vOrigin, _fvector vDir, _fmatrix WorldMatrix, _float3* pOutHit, float* pOutDist)
{
	if (iMeshIndex >= m_iNumMeshes) return false;

	// 레이를 로컬 공간으로 변환
	_matrix InvWorld = XMMatrixInverse(nullptr, WorldMatrix);

	_vector vLocalOrigin = XMVector3TransformCoord(vOrigin, InvWorld);
	_vector vLocalDir = XMVector3TransformNormal(vDir, InvWorld);
	vLocalDir = XMVector3Normalize(vLocalDir);

	_float3 localHit = {};
	float   dist = 0.f;

	if (!m_Meshes[iMeshIndex]->Ray_AABB(vLocalOrigin, vLocalDir)) return false;
	if (!m_Meshes[iMeshIndex]->Pick(vLocalOrigin, vLocalDir, &localHit, &dist)) return false;

	// 히트 포인트를 월드 공간으로 역변환
	if (pOutHit)
		XMStoreFloat3(pOutHit, XMVector3TransformCoord(XMLoadFloat3(&localHit), WorldMatrix));
	if (pOutDist)
		*pOutDist = dist;

	return true;
}

HRESULT CModel::Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix, PickableFilter fcFillter)
{
	m_eType = eType;
	m_PickableFilter = fcFillter;
	m_strModelPath = pModelFilePath ? StrToWstr(pModelFilePath) : L"" ;

	return m_eType == MODEL::ANIM ? Ready_Anim(pModelFilePath, PreTransformMatrix) : Ready_NonAnim(pModelFilePath, PreTransformMatrix);
}

HRESULT CModel::Initialize_Prototype_WithTextureHub(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix, PickableFilter fcFillter, _bool bCookCollisionMesh)
{
	m_eType = eType;
	m_bUseMaterialEx = true;
	m_PickableFilter = fcFillter;
	m_strModelPath = pModelFilePath ? StrToWstr(pModelFilePath) : L"";
	m_bCookCollisionMesh = bCookCollisionMesh;

	if (MODEL::ANIM == m_eType && m_bCookCollisionMesh)
		return E_FAIL;	// 현재 CookCollMesh는 NonAnimMesh만 지원

	return MODEL::ANIM == m_eType
		? Ready_AnimEx(pModelFilePath, PreTransformMatrix)
		: Ready_NonAnimEx(pModelFilePath, PreTransformMatrix);
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

void CModel::Set_AnimationIndex(_uint iIndex, _bool isLoop, _bool isRestart, _float fBlendDuration)
{
	_uint iPlayingIndex = m_isBlending ? m_iBlendTargetAnimIndex : m_iCurrentAnimationIndex;
	if (iPlayingIndex == iIndex)
	{
		if (isRestart && !m_Animations.empty())
			m_Animations[iPlayingIndex]->Reset_TrackPosition();
		m_isAnimLoop = isLoop;
		return;
	}

	if (m_iCurrentAnimationIndex == (_uint)-1 || fBlendDuration <= 0.f)
	{
		m_iCurrentAnimationIndex = iIndex;
		m_isAnimLoop = isLoop;
		m_Animations[iIndex]->Reset_TrackPosition();
		return;
	}

	m_iBlendTargetAnimIndex = iIndex;
	m_fBlendDuration = fBlendDuration;
	m_fBlendElapsed = 0.f;
	m_isAnimLoop = isLoop;
	m_isBlending = true;

	m_Animations[m_iBlendTargetAnimIndex]->Reset_TrackPosition();
}

_int CModel::Get_AnimationIndex(const string& strName) const
{
	for (_uint i = 0; i < m_iNumAnimations; ++i)
		if (m_Animations[i]->Get_AnimationName() == strName)
			return (_int)i;
	return -1;
}

const string& CModel::Get_CurrentAnimName() const
{
	static const string strEmpty = {};
	_uint idx = m_isBlending ? m_iBlendTargetAnimIndex : m_iCurrentAnimationIndex;
	if (idx == (_uint)-1 || idx >= m_Animations.size())
		return strEmpty;
	return m_Animations[idx]->Get_AnimationName();
}

void CModel::Seek_Animation(_float fProgress)
{
	_uint idx = m_isBlending ? m_iBlendTargetAnimIndex : m_iCurrentAnimationIndex;
	if (idx == (_uint)-1 || idx >= m_Animations.size())
		return;

	m_Animations[idx]->Set_Progress(fProgress);
	m_Animations[idx]->Update_TransformationMatrices(m_Bones, 0.f, false);

	Update_Combined();
}

_bool CModel::Update_Base(_float fTimeDelta, _float fSpeed)
{
	_bool isFinished = { false };

	if (m_isBlending)
	{
		m_fBlendElapsed += fTimeDelta;
		_float t = min(m_fBlendElapsed / m_fBlendDuration, 1.f);

		unordered_map<_uint, KEYFRAME> SrcMap, DstMap;
		m_Animations[m_iCurrentAnimationIndex]->Compute_BoneKeyFrames(SrcMap, fTimeDelta, false, fSpeed);
		m_Animations[m_iBlendTargetAnimIndex]->Compute_BoneKeyFrames(DstMap, fTimeDelta, false, fSpeed);

		auto MakeBindKeyFrame = [&](_uint boneIdx) -> KEYFRAME {
			KEYFRAME kf = {};
			_vector vS, vR, vT;
			XMMatrixDecompose(&vS, &vR, &vT, XMLoadFloat4x4(m_Bones[boneIdx]->Get_BindPoseMatrixPtr()));
			XMStoreFloat3(&kf.vScale, vS);
			XMStoreFloat4(&kf.vRotation, vR);
			XMStoreFloat3(&kf.vTranslation, vT);
			return kf;
			};

		unordered_set<_uint> BoneIndices;

		for (auto& [idx, _] : SrcMap)
			BoneIndices.insert(idx);

		for (auto& [idx, _] : DstMap)
			BoneIndices.insert(idx);

		for (_uint boneIdx : BoneIndices)
		{
			KEYFRAME SrcKF = SrcMap.count(boneIdx) ? SrcMap[boneIdx] : MakeBindKeyFrame(boneIdx);
			KEYFRAME DstKF = DstMap.count(boneIdx) ? DstMap[boneIdx] : MakeBindKeyFrame(boneIdx);

			_vector vS = XMVectorLerp(XMLoadFloat3(&SrcKF.vScale), XMLoadFloat3(&DstKF.vScale), t);

			_vector vR0 = XMLoadFloat4(&SrcKF.vRotation);
			_vector vR1 = XMLoadFloat4(&DstKF.vRotation);
			if (XMVectorGetX(XMVector4Dot(vR0, vR1)) < 0.f)
				vR1 = XMVectorNegate(vR1);
			_vector vR = XMQuaternionSlerp(vR0, vR1, t);

			_vector vT = XMVectorLerp(XMLoadFloat3(&SrcKF.vTranslation), XMLoadFloat3(&DstKF.vTranslation), t);

			KEYFRAME blended = {};
			XMStoreFloat3(&blended.vScale, vS);
			XMStoreFloat4(&blended.vRotation, vR);
			XMStoreFloat3(&blended.vTranslation, vT);

			_matrix TransformationMatrix = XMMatrixAffineTransformation(
				XMLoadFloat3(&blended.vScale),
				XMVectorSet(0.f, 0.f, 0.f, 1.f),
				XMLoadFloat4(&blended.vRotation),
				XMVectorSetW(XMLoadFloat3(&blended.vTranslation), 1.f));

			m_Bones[boneIdx]->Set_TransformationMatrix(TransformationMatrix);
		}

		if (t >= 1.f)
		{
			m_iCurrentAnimationIndex = m_iBlendTargetAnimIndex;
			m_isBlending = false;
		}
	}
	else
	{
		isFinished = m_Animations[m_iCurrentAnimationIndex]->Update_TransformationMatrices(m_Bones, fTimeDelta, m_isAnimLoop, fSpeed);
	}

	return isFinished;
}

_bool CModel::Play_Animation(_float fTimeDelta, _float fSpeed)
{
	_bool isFinished = Update_Base(fTimeDelta, fSpeed);
	Update_Combined();

	return isFinished;
}

_bool CModel::Play_Animation(_float fTimeDelta, _float fMaskTimeDelta, _int iMaskIndex, _float& fMaskLocalTime, vector<_uint>& MaskCursors, _float fSpeed, _float fMaskSpeed, _bool bLoop , _float fMaskWeight, _bool* pbOverlayFinished)
{
	_bool isFinished = Update_Base(fTimeDelta, fSpeed);
	_bool isOverlayFinished = Apply_Mask(iMaskIndex, fMaskLocalTime, MaskCursors, fMaskTimeDelta, fMaskSpeed, bLoop, fMaskWeight);
	if (pbOverlayFinished)
		*pbOverlayFinished = isOverlayFinished;

	Update_Combined();

	return isFinished;
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_Meshes[iMeshIndex]->Render()))
		return E_FAIL;

	return S_OK;
}

void CModel::Build_MaskBones(const vector<_string>& Roots)
{
	m_MaskBones.clear();

	_uint n = static_cast<_uint>(m_Bones.size());
	if (0 == n)
		return;

	// 1) Roots가 비면 전신 마스킹
	vector<char> masked(n, 0);
	if (Roots.empty())
	{
		for (_uint i = 0; i < n; ++i)
			masked[i] = 1;
	}
	else
	{
		for (const _string& strRoot : Roots)
		{
			_int iRoot = Get_BoneIndex(strRoot);
			if (iRoot >= 0)
				masked[iRoot] = 1;
		}
	}

	// 2) 부모 인덱스 < 자식 인덱스 가정하에 0부터 1패스 - 모든 루트 서브 트리를 동시에 합집합
	for (_uint i = 0; i < n; ++i)
	{
		if (masked[i])
			continue;
		_int p = Get_BoneParentIndex(i);
		if (p >= 0 && masked[p])
			masked[i] = 1;
	}

	// 3) 인덱스 리스트 수집 
	m_MaskBones.reserve(n);
	for (_uint i = 0; i < n; ++i)
	{
		if (masked[i])
			m_MaskBones.push_back(i);
	}
}

_bool CModel::Apply_Mask(_int iIndex, _float& fLocalTime, vector<_uint>& Cursors, _float fTimeDelta, _float fSpeed, _bool bLoop, _float fMaskWeight)
{
	if (fMaskWeight <= 0.f)
		return false;

	if (m_MaskBones.empty())
		return false;

	if (iIndex < 0 || iIndex >= static_cast<_int>(m_Animations.size()))
		return false;

	_bool isFinished = false;
	fLocalTime = m_Animations[iIndex]->Advance_Position(fLocalTime, fTimeDelta, bLoop, fSpeed, isFinished);

	unordered_map<_uint, KEYFRAME> poseMap;
	m_Animations[iIndex]->Sample_Pose(poseMap, fLocalTime, Cursors);

	auto Blend = [&](_uint iBone, const KEYFRAME& ov)
		{
			_vector vS = XMLoadFloat3(&ov.vScale);
			_vector vR = XMLoadFloat4(&ov.vRotation);
			_vector vT = XMLoadFloat3(&ov.vTranslation);

			if (fMaskWeight < 1.f)
			{
				_vector bS, bR, bT;
				XMMatrixDecompose(&bS, &bR, &bT, m_Bones[iBone]->Get_TransformationMatrix());			// 실시간 Base 포즈 기준

				if (XMVectorGetX(XMVector4Dot(bR, vR)) < 0.f)
					vR = XMVectorNegate(vR);
				vS = XMVectorLerp(bS, vS, fMaskWeight);
				vR = XMQuaternionSlerp(bR, vR, fMaskWeight);
				vT = XMVectorLerp(bT, vT, fMaskWeight);
			}
			m_Bones[iBone]->Set_TransformationMatrix(XMMatrixAffineTransformation(
				vS, XMVectorSet(0, 0, 0, 1), vR, XMVectorSetW(vT, 1.f)));
		};

	for (_uint iBone : m_MaskBones)
	{
		auto it = poseMap.find(iBone);
		if (it == poseMap.end())
			continue;

		Blend(iBone, it->second);
	}

	return isFinished;
}

void CModel::Update_Combined()
{
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformMatrices(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
}

HRESULT CModel::Bind_Material(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, MTEX_TYPE eType, _uint iIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	_uint iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	if (m_bUseMaterialEx)
	{
		if (iMaterialIndex >= m_MaterialsEx.size())
			return E_FAIL;

		return m_MaterialsEx[iMaterialIndex]->Bind_ShaderResource(pShader, pConstantName, eType, iIndex);
	}

	if (iMaterialIndex >= m_Materials.size())
		return E_FAIL;

	return m_Materials[iMaterialIndex]->Bind_ShaderResource(pShader, pConstantName, eType, iIndex);
}

HRESULT CModel::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	return m_Meshes[iMeshIndex]->Bind_BoneMatrices(pShader, pConstantName, m_Bones);
}

const MESH_LAYER_IDX& CModel::Get_MeshLayer(_uint iMesh) const
{
	static const MESH_LAYER_IDX kZero{};
	return (iMesh < m_MeshLayers.size()) ? m_MeshLayers[iMesh] : kZero;
}

void CModel::Set_MeshLayer(_uint iMesh, const MESH_LAYER_IDX& v)
{
	if (iMesh < m_MeshLayers.size())
		m_MeshLayers[iMesh] = v;
}

_uint CModel::Get_MeshTextureCount(_uint iMesh, MTEX_TYPE eType) const
{
	if (iMesh >= m_iNumMeshes) return 0u;

	const _uint iMaterialIndex = m_Meshes[iMesh]->Get_MaterialIndex();
	if (iMaterialIndex >= m_iNumMaterials) return 0u;

	if (m_bUseMaterialEx)
	{
		if (iMaterialIndex >= m_MaterialsEx.size() || nullptr == m_MaterialsEx[iMaterialIndex])
			return 0u;

		return m_MaterialsEx[iMaterialIndex]->Get_TextureCount(eType);
	}

	if (iMaterialIndex >= m_Materials.size() || nullptr == m_Materials[iMaterialIndex])
		return 0u;

	return m_Materials[iMaterialIndex]->Get_TextureCount(eType);
}

HRESULT CModel::Save_MeshLayers() const
{
	if (m_strMeshLayerPath.empty())
		return E_FAIL;

	json j;
	for (size_t i = 0; i < m_MeshLayers.size(); ++i)
	{
		const MESH_LAYER_IDX& m = m_MeshLayers[i];
		json jMesh;

		if (m.iPass >= 0)
			jMesh["Pass"] = m.iPass;

		if (m.iUVIndex != 0)
			jMesh["UVIndex"] = m.iUVIndex;

		if (m.iUnknownUVIndex != 0)
			jMesh["UnknownUVIndex"] = m.iUnknownUVIndex;

		const _bool bHasExtraUVIndex =
			m.iExtraUVIndex[0] != 0 ||
			m.iExtraUVIndex[1] != 0 ||
			m.iExtraUVIndex[2] != 0 ||
			m.iExtraUVIndex[3] != 0;

		if (bHasExtraUVIndex)
		{
			jMesh["ExtraUVIndex"] =
			{
				m.iExtraUVIndex[0],
				m.iExtraUVIndex[1],
				m.iExtraUVIndex[2],
				m.iExtraUVIndex[3]
			};
		}

		if (m.iFlags != 0)
			jMesh["Flags"] = m.iFlags;

		if (m.bUseUVTransform)
		{
			jMesh["UseUVTransform"] = true;
			jMesh["UVScale"] = { m.vUVScale.x, m.vUVScale.y };
			jMesh["UVScaleNormal"] = { m.vUVScaleNormal.x, m.vUVScaleNormal.y };
			jMesh["UVScaleMaterial"] = { m.vUVScaleMaterial.x, m.vUVScaleMaterial.y };
			jMesh["UVRotate"] = m.fUVRotate;
			jMesh["UVOffset"] = { m.vUVOffset.x, m.vUVOffset.y };
		}

		if (m.fNormalStrength != 1.f)
			jMesh["NormalStrength"] = m.fNormalStrength;

		if (m.fMaskStrength != 1.f)
			jMesh["MaskStrength"] = m.fMaskStrength;


		const unsigned int kUnknown = static_cast<unsigned int>(MTEX_TYPE::UNKNOWN);
		const _bool bHasExtraType =
			m.iExtraTexType[0] != kUnknown || m.iExtraTexType[1] != kUnknown ||
			m.iExtraTexType[2] != kUnknown || m.iExtraTexType[3] != kUnknown;

		if (bHasExtraType)
		{
			jMesh["ExtraTexType"] =
			{
					m.iExtraTexType[0], m.iExtraTexType[1],
					m.iExtraTexType[2], m.iExtraTexType[3]
			};
		}

		const _bool bHasExtraBind =
			m.iExtraBind[0] >= 0 ||
			m.iExtraBind[1] >= 0 ||
			m.iExtraBind[2] >= 0 ||
			m.iExtraBind[3] >= 0;

		if (bHasExtraBind)
		{
			jMesh["ExtraBind"] =
			{
				m.iExtraBind[0],
				m.iExtraBind[1],
				m.iExtraBind[2],
				m.iExtraBind[3]
			};
		}

		for (_uint t = 0; t < MTEX_TYPE_MAX; ++t)
		{
			if (m.idx[t] != 0)
				jMesh[to_string(t)] = m.idx[t];
		}

		if (Has_MeshLayerExValue(m))
		{
			jMesh["LayerEx"] = Save_MeshLayerEx(m);
		}

		if (!jMesh.empty())
			j[to_string(i)] = jMesh;
	}

	ofstream fout(m_strMeshLayerPath);
	if (!fout.is_open())
		return E_FAIL;

	fout << j.dump(2);
	return S_OK;
}

HRESULT CModel::Render_Instanced(_uint iMeshIndex, ID3D11Buffer* pInstanceBuffer, _uint iInstanceStride, _uint iInstanceCount)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources_Instanced(pInstanceBuffer, iInstanceStride)))
		return E_FAIL;

	return m_Meshes[iMeshIndex]->Render_Instanced(iInstanceCount);
}

HRESULT CModel::Ready_Meshes(const vector<MESH_DATA>& meshes, _fmatrix PreTransformMatrix)
{
	m_iNumMeshes = meshes.size();
	for (const auto& meshData : meshes)
	{
		_bool bPickable = m_PickableFilter ? m_PickableFilter(meshData.strName) : false;

		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, this, meshData, PreTransformMatrix, bPickable);
		if (nullptr == pMesh) return E_FAIL;
		m_Meshes.push_back(pMesh);
	}
	return S_OK;
}

HRESULT CModel::Ready_Materials(const vector<MATERIAL_DATA>& materials, const _char* pModelFilePath)
{
	if (m_bUseMaterialEx || !m_MaterialsEx.empty())
		return E_FAIL;

	m_iNumMaterials = materials.size();
	for (const auto& matData : materials)
	{
		CMaterial* pMaterial = CMaterial::Create(m_pDevice, m_pContext, matData, pModelFilePath);
		if (nullptr == pMaterial) return E_FAIL;
		m_Materials.push_back(pMaterial);
	}
	return S_OK;
}

HRESULT CModel::Ready_MaterialsEx(const vector<MATERIAL_DATA>& materials)
{
	if (!m_Materials.empty() || !m_MaterialsEx.empty())
		return E_FAIL;

	m_iNumMaterials = materials.size();

	for (const auto& matData : materials)
	{
		CMaterialEx* pMaterialEx = CMaterialEx::Create(matData);
		if (nullptr == pMaterialEx)
			return E_FAIL;

		m_MaterialsEx.push_back(pMaterialEx);
	}

	return S_OK;
}

HRESULT CModel::Ready_Bones(const vector<BONE_DATA>& bones)
{
	for (const auto& boneData : bones)
	{
		CBone* pBone = CBone::Create(boneData);
		if (nullptr == pBone) return E_FAIL;
		m_Bones.push_back(pBone);
	}
	return S_OK;
}

HRESULT CModel::Ready_Animations(const vector<ANIMATION_DATA>& animations)
{
	m_iNumAnimations = (_uint)animations.size();
	for (const auto& animData : animations)
	{
		CAnimation* pAnimation = CAnimation::Create(animData, this);
		if (nullptr == pAnimation) return E_FAIL;
		m_Animations.push_back(pAnimation);
	}
	return S_OK;
}

HRESULT CModel::Ready_NonAnimEx(const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	MODEL_DATA modelData;
	if (FAILED(CDataLoader::Read_ysh(StrToWstr(pModelFilePath).c_str(), modelData)))
		return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	if (FAILED(Ready_Meshes(modelData.Meshes, PreTransformMatrix)))
		return E_FAIL;

	if (MODEL::MAP == m_eType || m_bCookCollisionMesh)
	{
		if (FAILED(Cook_CollisionMesh(modelData.Meshes, PreTransformMatrix)))
			return E_FAIL;
	}

	if (FAILED(Ready_MaterialsEx(modelData.Materials)))
		return E_FAIL;

	Load_MeshLayers(pModelFilePath);

	return S_OK;
}

HRESULT CModel::Ready_AnimEx(const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	MODEL_DATA modelData;

	if (FAILED(CDataLoader::Read_ysh(StrToWstr(pModelFilePath).c_str(), modelData)))
	{
		return E_FAIL;
	}

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	if (FAILED(Ready_Bones(modelData.Bones)))
		return E_FAIL;

	if (FAILED(Ready_Meshes(modelData.Meshes, PreTransformMatrix)))
		return E_FAIL;

	Load_MeshLayers(pModelFilePath);

	if (FAILED(Ready_MaterialsEx(modelData.Materials)))
		return E_FAIL;

	if (FAILED(Ready_Animations(modelData.Animations)))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Ready_NonAnim(const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	MODEL_DATA modelData;
	if (FAILED(CDataLoader::Read_ysh(StrToWstr(pModelFilePath).c_str(), modelData)))
		return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	if (FAILED(Ready_Meshes(modelData.Meshes, PreTransformMatrix)))
		return E_FAIL;

	if (MODEL::MAP == m_eType)
		if (FAILED(Cook_CollisionMesh(modelData.Meshes, PreTransformMatrix)))
			return E_FAIL;

	if (FAILED(Ready_Materials(modelData.Materials, pModelFilePath)))
		return E_FAIL;

	Load_MeshLayers(pModelFilePath);

	return S_OK;
}

HRESULT CModel::Ready_Anim(const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	MODEL_DATA modelData;
	if (FAILED(CDataLoader::Read_ysh(StrToWstr(pModelFilePath).c_str(), modelData)))
		return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	if (FAILED(Ready_Bones(modelData.Bones)))
		return E_FAIL;

	if (FAILED(Ready_Meshes(modelData.Meshes, PreTransformMatrix)))
		return E_FAIL;

	Load_MeshLayers(pModelFilePath);

	if (FAILED(Ready_Materials(modelData.Materials, pModelFilePath)))
		return E_FAIL;

	if (FAILED(Ready_Animations(modelData.Animations)))
		return E_FAIL;

	return S_OK;
}

void CModel::Load_MeshLayers(const _char* pModelFilePath)
{
	m_MeshLayers.assign(m_iNumMeshes, MESH_LAYER_IDX{});

	string path = pModelFilePath;
	if (size_t dot = path.rfind('.'); dot != string::npos)
		path = path.substr(0, dot);
	path += "_meshlayer.json";
	m_strMeshLayerPath = path;

	ifstream fin(path);
	if (!fin.is_open())
		return;

	json j;
	try { fin >> j; }
	catch (...) { return; }

	if (!j.is_object())
		return;

	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		const auto MeshIter = j.find(to_string(i));

		if (MeshIter == j.end() || !MeshIter->is_object())
			continue;

		const json& jMesh = *MeshIter;
		MESH_LAYER_IDX& Layer = m_MeshLayers[i];

		if (jMesh.contains("Pass") && jMesh["Pass"].is_number_integer())
			Layer.iPass = jMesh["Pass"].get<int>();

		if (jMesh.contains("UVIndex") && jMesh["UVIndex"].is_number_integer())
		{
			const int iUVIndex = jMesh["UVIndex"].get<int>();
			if (0 <= iUVIndex && iUVIndex <= 3)
				Layer.iUVIndex = static_cast<_uint>(iUVIndex);
		}

		if (jMesh.contains("UnknownUVIndex") && jMesh["UnknownUVIndex"].is_number_integer())
		{
			const int iValue = jMesh["UnknownUVIndex"].get<int>();
			if (0 <= iValue && iValue <= 3)
				Layer.iUnknownUVIndex = static_cast<_uint>(iValue);
		}

		if (jMesh.contains("ExtraUVIndex")
			&& jMesh["ExtraUVIndex"].is_array()
			&& jMesh["ExtraUVIndex"].size() == 4)
		{
			for (_uint c = 0; c < 4; ++c)
			{
				if (!jMesh["ExtraUVIndex"][c].is_number_integer())
					continue;

				const int iValue = jMesh["ExtraUVIndex"][c].get<int>();
				if (0 <= iValue && iValue <= 3)
					Layer.iExtraUVIndex[c] = static_cast<_uint>(iValue);
			}
		}

		if (jMesh.contains("Flags") && jMesh["Flags"].is_number_integer())
		{
			const int iFlags = jMesh["Flags"].get<int>();
			if (0 <= iFlags)
				Layer.iFlags = static_cast<_uint>(iFlags);
		}

		if (jMesh.contains("UseUVTransform") && jMesh["UseUVTransform"].is_boolean())
		{
			Layer.bUseUVTransform = jMesh["UseUVTransform"].get<bool>();

			if (Layer.bUseUVTransform)
			{
				if (jMesh.contains("UVScale") && jMesh["UVScale"].is_array() && jMesh["UVScale"].size() == 2)
				{
					Layer.vUVScale.x = jMesh["UVScale"][0].get<_float>();
					Layer.vUVScale.y = jMesh["UVScale"][1].get<_float>();
				}
				if (jMesh.contains("UVScaleNormal") && jMesh["UVScaleNormal"].is_array() && jMesh["UVScaleNormal"].size() == 2)
				{
					Layer.vUVScaleNormal.x = jMesh["UVScaleNormal"][0].get<_float>();
					Layer.vUVScaleNormal.y = jMesh["UVScaleNormal"][1].get<_float>();
				}
				if (jMesh.contains("UVScaleMaterial") && jMesh["UVScaleMaterial"].is_array() && jMesh["UVScaleMaterial"].size() == 2)
				{
					Layer.vUVScaleMaterial.x = jMesh["UVScaleMaterial"][0].get<_float>();
					Layer.vUVScaleMaterial.y = jMesh["UVScaleMaterial"][1].get<_float>();
				}

				if (jMesh.contains("UVRotate") && jMesh["UVRotate"].is_number())
					Layer.fUVRotate = jMesh["UVRotate"].get<_float>();

				if (jMesh.contains("UVOffset") && jMesh["UVOffset"].is_array() && jMesh["UVOffset"].size() == 2)
				{
					Layer.vUVOffset.x = jMesh["UVOffset"][0].get<_float>();
					Layer.vUVOffset.y = jMesh["UVOffset"][1].get<_float>();
				}
			}
		}

		if (jMesh.contains("NormalStrength") && jMesh["NormalStrength"].is_number())
			Layer.fNormalStrength = jMesh["NormalStrength"].get<_float>();

		if (jMesh.contains("MaskStrength") && jMesh["MaskStrength"].is_number())
			Layer.fMaskStrength = jMesh["MaskStrength"].get<_float>();

		if (jMesh.contains("ExtraTexType") && jMesh["ExtraTexType"].is_array() && jMesh["ExtraTexType"].size() == 4)
		{
			for (_uint c = 0; c < 4; ++c)
			{
				if (jMesh["ExtraTexType"][c].is_number_integer())
					Layer.iExtraTexType[c] = jMesh["ExtraTexType"][c].get<unsigned int>();
			}
		}

		if (jMesh.contains("ExtraBind") && jMesh["ExtraBind"].is_array() && jMesh["ExtraBind"].size() == 4)
		{
			for (_uint c = 0; c < 4; ++c)
			{
				if (!jMesh["ExtraBind"][c].is_number_integer())
					continue;

				const int iValue = jMesh["ExtraBind"][c].get<int>();
				Layer.iExtraBind[c] = (iValue >= 0) ? iValue : -1;
			}
		}

		for (_uint t = 0; t < MTEX_TYPE_MAX; ++t)
		{
			const auto TextureIter = jMesh.find(to_string(t));

			if (TextureIter == jMesh.end())
				continue;

			if (TextureIter->is_number_unsigned())
			{
				Layer.idx[t] = TextureIter->get<_uint>();
			}
			else if (TextureIter->is_number_integer())
			{
				const _int iTextureIndex = TextureIter->get<_int>();

				if (0 <= iTextureIndex)
					Layer.idx[t] = static_cast<_uint>(iTextureIndex);
			}
		}

		const auto IterLayerEx = jMesh.find("LayerEx");
		if (IterLayerEx != jMesh.end())
		{
			Load_MeshLayerEx(*IterLayerEx, &Layer);
		}
	}
}

HRESULT CModel::Cook_CollisionMesh(const vector<MESH_DATA>& meshes, _fmatrix PreTransformMatrix)
{
	if (nullptr == m_pGameInstance_Proxy) return S_OK;

	vector<_float3> Positions;
	vector<_uint>   Indices;

	if (m_bCookCollisionMesh)
	{
		for (const auto& mesh : meshes) {
			const _uint iBase = (_uint)Positions.size();
			for (const auto& v : mesh.NonAnimVertices) {     // 환경은 NonAnimVertices
				_float3 p;
				XMStoreFloat3(&p, XMVector3TransformCoord(XMLoadFloat3(&v.vPosition), PreTransformMatrix));
				Positions.push_back(p);
			}
			for (_uint idx : mesh.Indices) Indices.push_back(iBase + idx);
		}
	}
	else
	{
		for (const auto& mesh : meshes) {
			const _uint iBase = (_uint)Positions.size();
			for (const auto& v : mesh.MapVertices) {     // MAP은 MapVertices
				_float3 p;
				XMStoreFloat3(&p, XMVector3TransformCoord(XMLoadFloat3(&v.vPosition), PreTransformMatrix));
				Positions.push_back(p);
			}
			for (_uint idx : mesh.Indices) Indices.push_back(iBase + idx);
		}
	}
	if (Positions.empty() || Indices.size() < 3) return S_OK;

	m_pCollisionMesh = m_pGameInstance_Proxy->Cook_TriangleMesh(
		Positions.data(), (_uint)Positions.size(),
		Indices.data(), (_uint)Indices.size(),
		false);
	return (nullptr != m_pCollisionMesh) ? S_OK : E_FAIL;
}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix, PickableFilter fcFillter)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix, fcFillter)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CModel* CModel::Create_WithTextureHub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType,
	const _char* pModelFilePath, _fmatrix PreTransformMatrix, PickableFilter fcFillter, _bool bCookCollisionMesh)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype_WithTextureHub(eType, pModelFilePath, PreTransformMatrix, fcFillter, bCookCollisionMesh)))
	{
		MSG_BOX("Failed to Created : CModel - WithTextureHub");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);
	m_Animations.clear();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);
	m_Bones.clear();

	if (m_pCollisionMesh)
	{
		m_pCollisionMesh->release();
		m_pCollisionMesh = nullptr;
	}

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);
	m_Materials.clear();

	for (auto& pMaterialEx : m_MaterialsEx)
		Safe_Release(pMaterialEx);
	m_MaterialsEx.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();
}
