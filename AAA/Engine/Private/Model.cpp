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
	, m_CollisionCookFilter{ Prototype.m_CollisionCookFilter }
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

_bool CModel::Get_CollisionAABB(_float3* pOutMin, _float3* pOutMax) const
{
	if (nullptr == pOutMin || nullptr == pOutMax || nullptr == m_pCollisionMesh)
		return false;

	const physx::PxBounds3 Bounds = m_pCollisionMesh->getLocalBounds();
	if (!Bounds.isValid() || Bounds.isEmpty())
		return false;

	*pOutMin =
	{
			Bounds.minimum.x,
			Bounds.minimum.y,
			Bounds.minimum.z
	};

	*pOutMax =
	{
			Bounds.maximum.x,
			Bounds.maximum.y,
			Bounds.maximum.z
	};

	return true;
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

HRESULT CModel::Initialize_Prototype_WithTextureHub(const CModel::MODEL_LOAD_DESC& Desc)
{
	m_eType = Desc.eType;
	m_bUseMaterialEx = true;
	m_PickableFilter = Desc.fcPickableFilter;
	m_strModelPath = Desc.pModelFilePath ? StrToWstr(Desc.pModelFilePath) : L"";
	m_bCookCollisionMesh = MODEL::MAP == m_eType ? false : Desc.bCookCollisionMesh;
	m_CollisionCookFilter = Desc.fcCollisionCookFilter;
	m_iCookExcludePass = Desc.iCookExcludePass;

	//if (MODEL::ANIM == m_eType && m_bCookCollisionMesh)
	//      return E_FAIL;  // 현재 CookCollMesh는 NonAnimMesh만 지원

	const _matrix PreTransformMatrix = XMLoadFloat4x4(&Desc.PreTransformMatrix);

	return MODEL::ANIM == m_eType
		? Ready_AnimEx(Desc.pModelFilePath, PreTransformMatrix)
		: Ready_NonAnimEx(Desc.pModelFilePath, PreTransformMatrix);
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
		m_isBlending = false;
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

void CModel::Set_BindPose()
{
	for (auto& pBone : m_Bones)
		pBone->Set_TransformationMatrix(XMLoadFloat4x4(pBone->Get_BindPoseMatrixPtr()));

	Update_Combined();
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

	// 인덱스 잘못되었을 때 가드 추가
	const _bool bInvalidBase =
		(m_iCurrentAnimationIndex == (_uint)-1 ||
			m_iCurrentAnimationIndex >= m_Animations.size());
	const _bool bInvalidBlend =
		(m_isBlending &&
			(m_iBlendTargetAnimIndex == (_uint)-1 ||
				m_iBlendTargetAnimIndex >= m_Animations.size()));

	if (bInvalidBase || bInvalidBlend)
	{
#ifdef _DEBUG
		static _bool s_bWarned = false;     
		if (!s_bWarned)
		{
			s_bWarned = true;
			MSG_BOX("CModel::Update_Base: animation index invalid "
				"(no clip playing). Update skipped.");
		}
#endif
		return false;                        
	}

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

void CModel::Build_MaskBones(const vector<_string>& Roots, vector<_uint>& OutBones)
{
	OutBones.clear();

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
	OutBones.reserve(n);
	for (_uint i = 0; i < n; ++i)
	{
		if (masked[i])
			OutBones.push_back(i);
	}
}

_bool CModel::Apply_Mask(LAYER& animLayer, _float fTimeDelta)
{
	if (animLayer.fWeight <= 0.f)
		return false;

	if (animLayer.MaskBones.empty())
		return false;

	if (animLayer.iAnimIndex < 0 || animLayer.iAnimIndex >= static_cast<_int>(m_Animations.size()))
		return false;

	const _float fDelta = animLayer.bPaused ? 0.f : fTimeDelta;		// pause 흡수

	_bool isFinished = false;
	animLayer.fLocalTime = m_Animations[animLayer.iAnimIndex]->Advance_Position(animLayer.fLocalTime, fDelta, animLayer.bLoop, animLayer.fSpeed, isFinished);

	unordered_map<_uint, KEYFRAME> curMap;
	m_Animations[animLayer.iAnimIndex]->Sample_Pose(curMap, animLayer.fLocalTime, animLayer.KeyFrameCursors);

	const _bool bCrossfade = (animLayer.bClipBlending
		&& animLayer.iPrevAnimIndex >= 0
		&& animLayer.iPrevAnimIndex < static_cast<_int>(m_Animations.size()));

	unordered_map<_uint, KEYFRAME> prevMap;
	_float t = 1.f;

	if (bCrossfade)
	{
		_bool prevFin = false;
		animLayer.fPrevLocalTime = m_Animations[animLayer.iPrevAnimIndex]->Advance_Position(
			animLayer.fPrevLocalTime, fDelta, animLayer.bPrevLoop, animLayer.fSpeed, prevFin);
		m_Animations[animLayer.iPrevAnimIndex]->Sample_Pose(prevMap, animLayer.fPrevLocalTime, animLayer.PrevCursors);

		animLayer.fClipBlendElapsed += fDelta;
		t = (animLayer.fClipBlend > 0.f) ? (animLayer.fClipBlendElapsed / animLayer.fClipBlend) : 1.f;
		if (t > 1.f)
			t = 1.f;
	}
	
	// clip crossfade Lerp (Prev -> Cur)
	auto LerpKF = [](const KEYFRAME& a, const KEYFRAME& b, _float r) -> KEYFRAME
		{
			_vector aS = XMLoadFloat3(&a.vScale), bS = XMLoadFloat3(&b.vScale);
			_vector aR = XMLoadFloat4(&a.vRotation), bR = XMLoadFloat4(&b.vRotation);
			_vector aT = XMLoadFloat3(&a.vTranslation), bT = XMLoadFloat3(&b.vTranslation);

			if (XMVectorGetX(XMVector4Dot(aR, bR)) < 0.f)
				bR = XMVectorNegate(bR);

			KEYFRAME o{};
			XMStoreFloat3(&o.vScale, XMVectorLerp(aS, bS, r));
			XMStoreFloat4(&o.vRotation, XMQuaternionSlerp(aR, bR, r));
			XMStoreFloat3(&o.vTranslation, XMVectorLerp(aT, bT, r));
			return o;
		};

	// overlay 포즈 -> Base에 Weight 블렌딩으로 마스킹

	auto BlendToBone = [&](_uint iBone, const KEYFRAME& ov)
		{
			_vector vS = XMLoadFloat3(&ov.vScale);
			_vector vR = XMLoadFloat4(&ov.vRotation);
			_vector vT = XMLoadFloat3(&ov.vTranslation);

			if (animLayer.fWeight < 1.f)
			{
				_vector bS, bR, bT;																		// TODO : SRT 중에 선택 받아서 보간 대상 결정하기
				XMMatrixDecompose(&bS, &bR, &bT, m_Bones[iBone]->Get_TransformationMatrix());			// 실시간 Base 포즈 기준

				if (XMVectorGetX(XMVector4Dot(bR, vR)) < 0.f)
					vR = XMVectorNegate(vR);
				vS = XMVectorLerp(bS, vS, animLayer.fWeight);
				vR = XMQuaternionSlerp(bR, vR, animLayer.fWeight);
				vT = XMVectorLerp(bT, vT, animLayer.fWeight);
			}
			m_Bones[iBone]->Set_TransformationMatrix(XMMatrixAffineTransformation(
				vS, XMVectorSet(0, 0, 0, 1), vR, XMVectorSetW(vT, 1.f)));
		};

	for (_uint iBone : animLayer.MaskBones)
	{
		auto it = curMap.find(iBone);
		if (it == curMap.end())
			continue;

		KEYFRAME ov = it->second;
		if (bCrossfade)
		{
			auto itp = prevMap.find(iBone);
			if (itp != prevMap.end())
				ov = LerpKF(itp->second, ov, t);
		}
		BlendToBone(iBone, ov);
	}

	// Crossfade 완료 -> Prev 폐기
	if (bCrossfade && t >= 1.f)
	{
		animLayer.bClipBlending = false;
		animLayer.iPrevAnimIndex = -1;
		animLayer.PrevCursors.clear();
	}

	return isFinished;
}

void CModel::RotateBone(const _char* szBone, _float fAngleDeg, _fvector vAxis)
{
	_int iBone = Get_BoneIndex(szBone);
	if (iBone < 0)
		return;

	_vector vScale, vRot, vTrans;
	XMMatrixDecompose(&vScale, &vRot, &vTrans, m_Bones[iBone]->Get_TransformationMatrix());
	_vector qAdd = XMQuaternionRotationAxis(XMVector3Normalize(vAxis), XMConvertToRadians(fAngleDeg));

	vRot = XMQuaternionNormalize(XMQuaternionMultiply(vRot, qAdd));
	m_Bones[iBone]->Set_TransformationMatrix(XMMatrixAffineTransformation(vScale, XMVectorSet(0, 0, 0, 1), vRot, XMVectorSetW(vTrans, 1.f)));
}

void CModel::Update_Combined()
{
	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformMatrices(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
}

_float CModel::Get_AnimationDuration(_uint iIndex) const
{
	return iIndex < m_Animations.size() ? m_Animations[iIndex]->Get_Duration() : 0.f;
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
		const json jMesh = Save_MeshLayer(m_MeshLayers[i]);
		if (!jMesh.empty())
			j[to_string(i)] = jMesh;
	}

	_string strInvalidPath;
	if (JsonUtils::Find_NonFiniteNumberPath(j, {}, &strInvalidPath))
	{
		OutputDebugStringA("[MeshLayer] Save failed: non-finite value at ");
		OutputDebugStringA(strInvalidPath.empty() ? "<root>" : strInvalidPath.c_str());
		OutputDebugStringA("\n");
		return E_FAIL;
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

	Load_MeshLayers(pModelFilePath);

	if (MODEL::MAP == m_eType || m_bCookCollisionMesh)
	{
		if (FAILED(Cook_CollisionMesh(modelData.Meshes, PreTransformMatrix)))
			return E_FAIL;
	}

	if (FAILED(Ready_MaterialsEx(modelData.Materials)))
		return E_FAIL;

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

	if (m_bCookCollisionMesh)
	{
		if (FAILED(Cook_CollisionAnimMesh(modelData.Meshes, PreTransformMatrix)))
			return E_FAIL;
	}

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

		MESH_LAYER_IDX Layer{};
		if (SUCCEEDED(Load_MeshLayer(*MeshIter, &Layer)))
			m_MeshLayers[i] = Layer;
	}
}

_bool CModel::Should_CookCollisionMesh(_uint iMeshIndex, const string& strMeshName) const
{
	if (m_iCookExcludePass >= 0 && iMeshIndex < m_MeshLayers.size() && m_iCookExcludePass == m_MeshLayers[iMeshIndex].iPass)
		return false;

	if (MODEL::MAP != m_eType)
		return true;

	return !m_CollisionCookFilter || m_CollisionCookFilter(strMeshName);
}

HRESULT CModel::Cook_CollisionMesh(const vector<MESH_DATA>& meshes, _fmatrix PreTransformMatrix)
{
	if (nullptr == m_pGameInstance_Proxy) return S_OK;

	vector<_float3> Positions;
	vector<_uint>   Indices;

	if (m_bCookCollisionMesh)
	{
		_uint iMeshIndex = 0;
		for (const auto& mesh : meshes) {
			if (!Should_CookCollisionMesh(iMeshIndex++, mesh.strName))
				continue;

			const _uint iBase = (_uint)Positions.size();
			for (const auto& v : mesh.NonAnimVertices) {
				_float3 p;
				XMStoreFloat3(&p, XMVector3TransformCoord(XMLoadFloat3(&v.vPosition), PreTransformMatrix));
				Positions.push_back(p);
			}
			for (_uint idx : mesh.Indices) Indices.push_back(iBase + idx);
		}
	}
	else
	{
		_uint iMeshIndex = 0;
		for (const auto& mesh : meshes) {
			if (!Should_CookCollisionMesh(iMeshIndex++, mesh.strName))
				continue;

			const _uint iBase = (_uint)Positions.size();
			for (const auto& v : mesh.MapVertices) {
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

HRESULT CModel::Cook_CollisionAnimMesh(const vector<MESH_DATA>& meshes, _fmatrix PreTransformMatrix)
{
	if (nullptr == m_pGameInstance_Proxy) return S_OK;
	if (MODEL::ANIM != m_eType) return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
	Update_Combined();

	vector<_float3> Positions;
	vector<_uint> Indices;

	auto BuildMeshBoneMatrices = [&](const MESH_DATA& mesh, vector<_float4x4>* pOutMatrices) -> HRESULT
		{
			pOutMatrices->clear();

			if (mesh.Bones.empty())
			{
				const _int iBoneIndex = Get_BoneIndex(mesh.strName);
				if (-1 == iBoneIndex) return E_FAIL;

				_float4x4 BoneMatrix{};
				XMStoreFloat4x4(&BoneMatrix, XMLoadFloat4x4(m_Bones[iBoneIndex]->Get_CombinedTransformationMatrixPtr()));
				pOutMatrices->push_back(BoneMatrix);
				return S_OK;
			}

			for (const MESH_BONE_DATA& BoneData : mesh.Bones)
			{
				const _int iBoneIndex = Get_BoneIndex(BoneData.strName);
				if (-1 == iBoneIndex) return E_FAIL;

				_float4x4 BoneMatrix{};
				XMStoreFloat4x4(&BoneMatrix,
					XMLoadFloat4x4(&BoneData.OffsetMatrix) *
					XMLoadFloat4x4(m_Bones[iBoneIndex]->Get_CombinedTransformationMatrixPtr()));
				pOutMatrices->push_back(BoneMatrix);
			}

			return S_OK;
		};

	auto TransformSkinnedPosition = [](const VTXANIMMESH_DATA& Vertex, const vector<_float4x4>& BoneMatrices, _float3* pOutPosition) -> _bool
		{
			const _uint BlendIndices[4] = { Vertex.vBlendIndex.x, Vertex.vBlendIndex.y, Vertex.vBlendIndex.z, Vertex.vBlendIndex.w };
			const _float fWeightW = 1.f - (Vertex.vBlendWeight.x + Vertex.vBlendWeight.y + Vertex.vBlendWeight.z);
			const _float BlendWeights[4] = { Vertex.vBlendWeight.x, Vertex.vBlendWeight.y, Vertex.vBlendWeight.z, fWeightW };

			_vector vSource = XMLoadFloat3(&Vertex.vPosition);
			_vector vResult = XMVectorZero();

			for (_uint i = 0; i < 4; ++i)
			{
				if (fabsf(BlendWeights[i]) <= 1e-6f)
					continue;
				if (BlendIndices[i] >= BoneMatrices.size())
					return false;

				const _vector vSkinned = XMVector3TransformCoord(vSource, XMLoadFloat4x4(&BoneMatrices[BlendIndices[i]]));
				vResult = XMVectorAdd(vResult, XMVectorScale(vSkinned, BlendWeights[i]));
			}

			XMStoreFloat3(pOutPosition, vResult);
			return true;
		};

	for (const MESH_DATA& mesh : meshes)
	{
		vector<_float4x4> MeshBoneMatrices;
		if (FAILED(BuildMeshBoneMatrices(mesh, &MeshBoneMatrices)))
			return E_FAIL;

		const _uint iBase = static_cast<_uint>(Positions.size());

		for (const VTXANIMMESH_DATA& Vertex : mesh.AnimVertices)
		{
			_float3 vPosition{};
			if (!TransformSkinnedPosition(Vertex, MeshBoneMatrices, &vPosition))
				return E_FAIL;

			Positions.push_back(vPosition);
		}

		if (iBase == static_cast<_uint>(Positions.size()))
			continue;

		for (_uint iIndex : mesh.Indices)
			Indices.push_back(iBase + iIndex);
	}

	if (Positions.empty() || Indices.size() < 3) return S_OK;

	m_pCollisionMesh = m_pGameInstance_Proxy->Cook_TriangleMesh(
		Positions.data(), static_cast<_uint>(Positions.size()),
		Indices.data(), static_cast<_uint>(Indices.size()),
		false);

	return nullptr != m_pCollisionMesh ? S_OK : E_FAIL;
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

CModel* CModel::Create_WithTextureHub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CModel::MODEL_LOAD_DESC& Desc)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype_WithTextureHub(Desc)))
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
