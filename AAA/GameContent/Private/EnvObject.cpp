#include "EnvObject.h"
#include "MeshLayer_Binder.h"

#include "GameInstance.h"
#include "Profiler_Manager.h"

NS_BEGIN(Client)

namespace
{
	constexpr _bool		ENABLE_ENV_OBJECT_SHADOW = true;
	constexpr _float	ENV_DISTANCE_CULL_START = 175.f;
	constexpr _float	ENV_SHADOW_DISTANCE_CULL_START = 175.f;
	constexpr _float	ENV_PICK_AABB_PADDING = 0.05f;
	constexpr _float	ENV_PICK_THIN_EXTENT = 0.06f;

	void Log_EnvPhysicsWarning(const string& strMessage)
	{
		OutputDebugStringA((strMessage + "\n").c_str());
	}

#ifdef _DEBUG
	void Log_EnvPhysicsInfo(const string& strMessage)
	{
		OutputDebugStringA((strMessage + "\n").c_str());
	}
#endif

	_matrix Build_WorldMatrix_FromTRS(const ENV_OBJECT_DESC& Desc)
	{
		const _vector vScale = XMLoadFloat3(&Desc.vScale);
		const _vector vRotation = XMLoadFloat4(&Desc.vRotation);
		const _vector vPosition = XMVectorSet(Desc.vPosition.x, Desc.vPosition.y, Desc.vPosition.z, 1.f);

		return XMMatrixScalingFromVector(vScale)
			* XMMatrixRotationQuaternion(vRotation)
			* XMMatrixTranslationFromVector(vPosition);
	}

	BoundingBox Make_DefaultAABB()
	{
		BoundingBox Bounds{};
		Bounds.Center = _float3(0.f, 0.f, 0.f);
		Bounds.Extents = _float3(0.5f, 0.5f, 0.5f);
		return Bounds;
	}

	_bool Is_FiniteFloat(_float fValue)
	{
		return std::isfinite(fValue);
	}

	_bool Is_ValidAABB(const _float3& vMin, const _float3& vMax)
	{
		return Is_FiniteFloat(vMin.x) && Is_FiniteFloat(vMin.y) && Is_FiniteFloat(vMin.z)
			&& Is_FiniteFloat(vMax.x) && Is_FiniteFloat(vMax.y) && Is_FiniteFloat(vMax.z)
			&& vMax.x >= vMin.x && vMax.y >= vMin.y && vMax.z >= vMin.z;
	}

	BoundingBox Make_AABB_FromMinMax(const _float3& vMin, const _float3& vMax)
	{
		BoundingBox Bounds{};
		Bounds.Center = _float3(
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f);
		Bounds.Extents = _float3(
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f);
		return Bounds;
	}

	_bool Is_ThinBounds(const BoundingBox& Bounds)
	{
		return Bounds.Extents.x <= ENV_PICK_THIN_EXTENT
			|| Bounds.Extents.y <= ENV_PICK_THIN_EXTENT
			|| Bounds.Extents.z <= ENV_PICK_THIN_EXTENT;
	}

	_bool Is_ThinObjectBounds(const BoundingBox& LocalBounds, const _float3& vObjectScale)
	{
		BoundingBox ScaledBounds = LocalBounds;
		ScaledBounds.Extents.x *= vObjectScale.x;
		ScaledBounds.Extents.y *= vObjectScale.y;
		ScaledBounds.Extents.z *= vObjectScale.z;

		return Is_ThinBounds(ScaledBounds);
	}

	_float3 RayPoint(_fvector vOrigin, _fvector vDir, _float fDist)
	{
		_float3 vPoint = {};

		const _vector vWorldPoint =
			XMVectorAdd(vOrigin, XMVectorScale(XMVector3Normalize(vDir), fDist));

		XMStoreFloat3(&vPoint, vWorldPoint);
		return vPoint;
	}
}

CEnvObject::CEnvObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{pDevice, pContext}
	, m_bRenderable{ false }
	, m_bCastShadow{ false }
	, m_bUseCullDistance{ true }
	, m_bUseCullFrustum{ true }
	, m_bIsDecal{ false }
	, m_fDecalAlpha{ 1.f }
{
}

CEnvObject::CEnvObject(const CEnvObject& Prototype)
	: CGameObject(Prototype)
	, m_bRenderable{ Prototype.m_bRenderable }
	, m_bCastShadow{ Prototype.m_bCastShadow }
	, m_bUseCullDistance{ Prototype.m_bUseCullDistance }
	, m_bUseCullFrustum{ Prototype.m_bUseCullFrustum }
	, m_bIsDecal{ Prototype.m_bIsDecal }
	, m_fDecalAlpha{ Prototype.m_fDecalAlpha }
	, m_tDesc(Prototype.m_tDesc)
	, m_strProtoTag(Prototype.m_strProtoTag)
{
}

HRESULT CEnvObject::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	m_iMaterialID = WORLD_STATIC_ID;
	return S_OK;
}

HRESULT CEnvObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const ENV_OBJECT_DESC* pDesc = static_cast<const ENV_OBJECT_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tDesc = *pDesc;
	Apply_DescDefaults();
	Apply_TransformFromDesc();

	m_bUseCameraDither = m_tDesc.tRender.bUseNearDistAlpha;
	m_bIsDecal = m_tDesc.tRender.bIsDecal;

	return S_OK;
}

void CEnvObject::Late_Update(_float fTimeDelta)
{
	if (!m_bUseCameraDither) { m_fDissolve = 0.f; return; }

	_vector C = XMLoadFloat4(m_pGameInstance_Proxy->Get_CamPosition());
	_vector E = XMLoadFloat3(&m_WorldBounds.Center);   // 객체 위치
	_float  d = XMVectorGetX(XMVector3Length(E - C));  // 객체-카메라 거리

	// near → 1(사라짐),  far → 0(불투명)
	_float t = (m_fDitherFar - d) / max(m_fDitherFar - m_fDitherNear, 1e-4f);
	m_fDissolve = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
}

HRESULT CEnvObject::Render()
{
	if (!m_bRenderable)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (m_bUseCameraDither && m_fDissolve >= 0.999f)
			continue;

		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.pShader = m_pShaderCom;
		Ctx.pModel = m_pModelCom;
		Ctx.pGI_Proxy = m_pGameInstance_Proxy;
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::NONANIM_PBR;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = ShaderPass::NonAnimPBR::DMN;
		Ctx.fDissolve = m_fDissolve;

		if (m_bUseCameraDither)
			Ctx.iExtraFlags |= ShaderPass::EnvInstFlags::Dither;

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;

		if (Result.bSkipMesh)
			continue;

		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEnvObject::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.pShader = m_pShaderCom;
		Ctx.pModel = m_pModelCom;
		Ctx.pGI_Proxy = m_pGameInstance_Proxy;
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::NONANIM_PBR;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::SHADOW;
		Ctx.iFallbackPass = ShaderPass::NonAnimPBR::Shadow;

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;

		if (Result.bSkipMesh)
			continue;

		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEnvObject::Render_Decal()
{
	if (!m_bRenderable)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* -----------------------데칼 전용 추가----------------------- */
	_float4x4 WorldMatrixInverse{};
	XMStoreFloat4x4(&WorldMatrixInverse, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrixInverse", &WorldMatrixInverse)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
		return E_FAIL;

	const _float3 vDecalBoundsCenter = m_LocalBounds.Center;
	const _float3 vDecalBoundsExtents = m_LocalBounds.Extents;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vDecalBoundsCenter", &vDecalBoundsCenter, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vDecalBoundsExtents", &vDecalBoundsExtents, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fDecalAlpha", &m_fDecalAlpha, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MaterialID"), m_pShaderCom, "g_MaterialIDTexture")))
		return E_FAIL;

	const _int iDecalMaskMode = 1;                 // 0=제외, 1=한정 (추후 데칼 desc로 노출 가능)
	const _int iDecalMaskID = WORLD_STATIC_ID;	   // 1
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iDecalMaskMode", &iDecalMaskMode, sizeof(_int)))) return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iDecalMaskID", &iDecalMaskID, sizeof(_int)))) return E_FAIL;
	/* ------------------------------------------------------------ */

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.pShader = m_pShaderCom;
		Ctx.pModel = m_pModelCom;
		Ctx.pGI_Proxy = m_pGameInstance_Proxy;
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::NONANIM_PBR;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::DECAL;
		Ctx.iFallbackPass = ShaderPass::NonAnimPBR::DECAL;

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;
		if (Result.bSkipMesh)
			continue;

		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CEnvObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = m_strProtoTag;
}

_bool CEnvObject::Pick_Marb1e(_fvector vRayOrigin, _fvector vRayDir, _float3* pOutHit, _float* fOutDistance)
{
	if (!m_pModelCom)
		return false;

	_float3 closestHit = {};
	float   closestDist = FLT_MAX;
	bool    bHit = false;

	_fmatrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	float   dist = 0.f;


	_uint iCountMeshes = (_uint)m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iCountMeshes; ++i)
	{
		_float3 hit = {};

		if (m_pModelCom->Pick_Mesh(i, vRayOrigin, vRayDir, WorldMatrix, &hit, &dist))
		{
			string dbg = "HIT Mesh[" + to_string(i) + "]: "
				+ m_pModelCom->Get_MeshName(i) + "\n";
			OutputDebugStringA(dbg.c_str());

			if (dist < closestDist)
			{
				closestDist = dist;
				closestHit = hit;
				bHit = true;
			}
		}
	}

	if (bHit && pOutHit)
		*pOutHit = closestHit;

	if (dist && fOutDistance)
		*fOutDistance = closestDist;

	return bHit;
}

HRESULT CEnvObject::Ready_RenderComponents(_uint iModelProtoLevel, const wstring& wstrModelProtoTag)
{
	if (wstrModelProtoTag.empty())
	{
		if(!m_bRenderable)
			return S_OK;

		return E_FAIL;	// 렌더할 객체가 모델이 없으면 초기화 실패
	}

	m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(iModelProtoLevel, wstrModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	Update_LocalBounds();
	Refresh_WorldBounds();
	return S_OK;
}

HRESULT CEnvObject::Ready_PhysicsActor()
{
	Release_PhysicsActor();

	if (!Should_CreatePhysicsActor())
	{
//#ifdef _DEBUG
//		if (m_tDesc.tCollision.bInvalidCollision)
//		{
//			Log_EnvPhysicsInfo(
//				"[EnvPhysics] Skip actor: invalid collision. object="
//				+ WstrToStr(m_tDesc.wstrObjectName)
//				+ " uid="
//				+ to_string(m_tDesc.iUid)
//				+ " modelTag="
//				+ WstrToStr(m_tDesc.wstrModelProtoTag));
//		}
//#endif
		return S_OK;
	}

	switch (m_tDesc.tCollision.eColliderKind)
	{
	case ENV_COLLIDER_KIND::MODEL_MESH:
		return Ready_PhysicsActor_ModelMesh();

	case ENV_COLLIDER_KIND::SIMPLE_SHAPE:
		// 다음 단계에서 Cube / Sphere / Cylinder / Slope 처리 예정.
		return S_OK;

	case ENV_COLLIDER_KIND::NONE:
	case ENV_COLLIDER_KIND::TRIGGER_ONLY:
	case ENV_COLLIDER_KIND::UNKNOWN:
	default:
		return S_OK;
	}
}

HRESULT CEnvObject::Ready_PhysicsActor_ModelMesh()
{
	if (!m_tDesc.tCollision.bHasCollMesh)
	{
#ifdef _DEBUG
		Log_EnvPhysicsInfo(
			"[EnvPhysics] MODEL_MESH actor skipped : no collision mesh capability.object = "
			+ WstrToStr(m_tDesc.wstrObjectName)
			+ " uid="
			+ to_string(m_tDesc.iUid)
			+ " modelTag="
			+ WstrToStr(m_tDesc.wstrModelProtoTag));
#endif
		return S_OK;
	}

	if (nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	if (nullptr == m_pTransformCom)
		return E_FAIL;

	if (nullptr == m_pModelCom)
	{
#ifdef _DEBUG
		Log_EnvPhysicsWarning(
			"EnvObject MODEL_MESH collision skipped: model component is null.");
#endif
		return S_OK;
	}

	physx::PxTriangleMesh* pCollisionMesh = m_pModelCom->Get_CollisionMesh();
	if (nullptr == pCollisionMesh)
	{
#ifdef _DEBUG
		Log_EnvPhysicsWarning(
			"EnvObject MODEL_MESH collision skipped: cooked collision mesh is null.");
#endif
		return S_OK;
	}

	m_pPhysicsActor = m_pGameInstance_Proxy->Create_StaticActor(
		pCollisionMesh,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pPhysicsActor)
	{
#ifdef _DEBUG
		Log_EnvPhysicsWarning(
			"[EnvPhysics] MODEL_MESH actor failed: Create_StaticActor returned null. object="
			+ WstrToStr(m_tDesc.wstrObjectName)
			+ " uid="
			+ to_string(m_tDesc.iUid)
			+ " modelTag="
			+ WstrToStr(m_tDesc.wstrModelProtoTag));
#endif
		return E_FAIL;
	}

//#ifdef _DEBUG
//	Log_EnvPhysicsInfo(
//		"[EnvPhysics] MODEL_MESH actor created. object="
//		+ WstrToStr(m_tDesc.wstrObjectName)
//		+ " uid="
//		+ to_string(m_tDesc.iUid)
//		+ " apxbin="
//		+ WstrToStr(m_tDesc.tCollision.strDecorCollisionApxbinName)
//		+ " modelTag="
//		+ WstrToStr(m_tDesc.wstrModelProtoTag));
//#endif

	return S_OK;
}

void CEnvObject::Release_PhysicsActor()
{
	if (nullptr == m_pPhysicsActor)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pPhysicsActor);

	m_pPhysicsActor = nullptr;
}

_bool CEnvObject::Should_CreatePhysicsActor() const
{
	const ENV_COLLISION_DESC& Collision = m_tDesc.tCollision;

	switch (Collision.eColliderKind)
	{
	case ENV_COLLIDER_KIND::MODEL_MESH:
		if (m_tDesc.eKind == ENV_OBJECT_KIND::STATIC)
		{
			return Collision.bHasCollMesh
				&& Collision.bUseCollMesh;
		}

		// Interact 등은 이번 단위에서 기존 정책 유지.
		return Collision.bHasCollMesh
			&& !Collision.bInvalidCollision;

	case ENV_COLLIDER_KIND::SIMPLE_SHAPE:
	case ENV_COLLIDER_KIND::TRIGGER_ONLY:
		// 단순 충돌/트리거는 기존 원본 invalid 플래그를 존중한다.
		return !Collision.bInvalidCollision;

	case ENV_COLLIDER_KIND::NONE:
	case ENV_COLLIDER_KIND::UNKNOWN:
	default:
		return false;
	}
}

HRESULT CEnvObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CEnvObject::Update_LocalBounds()
{
	if (nullptr == m_pModelCom)
	{
		m_LocalBounds = Make_DefaultAABB();
		return;
	}

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (!Is_ValidAABB(vMin, vMax))
	{
		m_LocalBounds = Make_DefaultAABB();
		return;
	}

	m_LocalBounds = Make_AABB_FromMinMax(vMin, vMax);
}

void CEnvObject::Refresh_WorldBounds()
{
	if (nullptr == m_pTransformCom || !m_bTransformDirty)
		return;

	m_LocalBounds.Transform(m_WorldBounds, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	m_bTransformDirty = false;
}

void CEnvObject::Check_Visible()
{
	const _bool bEnableShadow = ENABLE_ENV_OBJECT_SHADOW && m_bCastShadow;

	m_bVisible = m_bRenderable;
	m_bVisibleShadow = bEnableShadow;

	// Distance -> Frustum
	// 1. Main
	if (m_bVisible)
	{
		if (m_bUseCullDistance && m_pGameInstance_Proxy->Should_CullByDistance(m_WorldBounds, ENV_DISTANCE_CULL_START))
		{
			m_bVisible = false;
		}
		else if (m_bUseCullFrustum && m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::MAIN_CAMERA, m_WorldBounds))
		{
			m_bVisible = false;
		}
	}

	// 2. Shadow
	if (m_bVisibleShadow)
	{
		if (m_bUseCullDistance && m_pGameInstance_Proxy->Should_CullByDistance(m_WorldBounds, ENV_SHADOW_DISTANCE_CULL_START))
		{
			m_bVisibleShadow = false;
		}
		else if (m_bUseCullFrustum && m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::SHADOW_DIR, m_WorldBounds))
		{
			m_bVisibleShadow = false;
		}
	}

#pragma region Profiling
	if (m_bVisible)
		PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_VISIBLE_MAIN, 1);

	if (m_bVisibleShadow)
		PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_VISIBLE_SHADOW, 1);
#pragma endregion
}

void CEnvObject::Apply_TransformFromDesc()
{
	if (nullptr == m_pTransformCom)
		return;

	if (m_tDesc.bHasWorldMatrix)
	{
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_tDesc.matWorld));
	}
	else
	{
		m_pTransformCom->Set_WorldMatrix(Build_WorldMatrix_FromTRS(m_tDesc));
	}

	m_bTransformDirty = true;

	Refresh_WorldBounds();
}

void CEnvObject::Apply_DescDefaults()
{
	m_bRenderable = !m_tDesc.tCollision.bInvisibleCollision;
	m_bUseCullDistance = m_tDesc.tRender.bUseCullDistance;
	m_bUseCullFrustum = m_tDesc.tRender.bUseCullFrustum;
	m_bCastShadow = m_tDesc.tRender.bHasShadow; //&& m_tDesc.tRender.bUseShadow;
	m_bVisible = true;
}

void CEnvObject::Free()
{
	Release_PhysicsActor();

	__super::Free();
}

NS_END
