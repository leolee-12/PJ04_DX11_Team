#include "EnvObject.h"
#include "GameContent_const.h"

#include "GameInstance_Proxy.h"
#include "Model.h"

#include <cmath>

NS_BEGIN(Client)

namespace
{
	constexpr _bool ENABLE_ENV_OBJECT_SHADOW = false;
	constexpr _float ENV_DISTANCE_CULL_START = 175.f;

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
}

CEnvObject::CEnvObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{pDevice, pContext}
{
}

CEnvObject::CEnvObject(const CEnvObject& Prototype)
	: CGameObject(Prototype)
	, m_tDesc(Prototype.m_tDesc)
	, m_strProtoTag(Prototype.m_strProtoTag)
{
}

HRESULT CEnvObject::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
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

	return S_OK;
}

void CEnvObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = m_strProtoTag;
}

HRESULT CEnvObject::Ready_RenderComponents(_uint iModelProtoLevel, const wstring& strModelProtoTag)
{
	if (strModelProtoTag.empty())
		return S_OK;

	m_pShaderCom = Add_Component<CShader>(
		Shader_NonAnimMesh_PBR.iLevelID,
		Shader_NonAnimMesh_PBR.szProtoTag,
		TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		iModelProtoLevel,
		strModelProtoTag,
		TEXT("Com_Model"));
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
#ifdef _DEBUG
		if (m_tDesc.tCollision.bInvalidCollision)
		{
			Log_EnvPhysicsInfo(
				"[EnvPhysics] Skip actor: invalid collision. object="
				+ WstrToStr(m_tDesc.strObjectName)
				+ " uid="
				+ to_string(m_tDesc.iUid)
				+ " modelTag="
				+ WstrToStr(m_tDesc.strModelProtoTag));
		}
#endif
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
	if (!m_tDesc.tCollision.bHasDecorCollisionApxbin)
	{
#ifdef _DEBUG
		Log_EnvPhysicsInfo(
			"[EnvPhysics] MODEL_MESH actor skipped: no decor collision apxbin. object="
			+ WstrToStr(m_tDesc.strObjectName)
			+ " uid="
			+ to_string(m_tDesc.iUid)
			+ " modelTag="
			+ WstrToStr(m_tDesc.strModelProtoTag));
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

	m_pPhysicsActor = m_pGameInstance_Proxy->Add_StaticActor(
		pCollisionMesh,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pPhysicsActor)
	{
#ifdef _DEBUG
		Log_EnvPhysicsWarning(
			"[EnvPhysics] MODEL_MESH actor failed: Add_StaticActor returned null. object="
			+ WstrToStr(m_tDesc.strObjectName)
			+ " uid="
			+ to_string(m_tDesc.iUid)
			+ " modelTag="
			+ WstrToStr(m_tDesc.strModelProtoTag));
#endif
		return E_FAIL;
	}

#ifdef _DEBUG
	Log_EnvPhysicsInfo(
		"[EnvPhysics] MODEL_MESH actor created. object="
		+ WstrToStr(m_tDesc.strObjectName)
		+ " uid="
		+ to_string(m_tDesc.iUid)
		+ " apxbin="
		+ WstrToStr(m_tDesc.tCollision.strDecorCollisionApxbinName)
		+ " modelTag="
		+ WstrToStr(m_tDesc.strModelProtoTag));
#endif

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
		// Decor 모델 메쉬 충돌은 카탈로그 hit가 최종 기준이다.
		return Collision.bHasDecorCollisionApxbin
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
	if (nullptr == m_pShaderCom || nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnvObject::Render()
{
	if (nullptr == m_pModelCom)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(static_cast<_uint>(i));

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", static_cast<_uint>(i), MTEX_TYPE::DIFFUSE,
			Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)])))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", static_cast<_uint>(i), MTEX_TYPE::NORMALS,
			Layer.idx[ETOUI(MTEX_TYPE::NORMALS)])))
			int a = 1;/*continue;*/
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", static_cast<_uint>(i), MTEX_TYPE::METALNESS,
			Layer.idx[ETOUI(MTEX_TYPE::METALNESS)])))
			int a = 1;/*continue;*/
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnknownTexture", static_cast<_uint>(i), MTEX_TYPE::UNKNOWN,
			Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)])))
			int a = 1;/*continue;*/

		_uint iPass = (Layer.iPass >= 0)
			? static_cast<_uint>(Layer.iPass)
			: ShaderPass::NonAnimPBR::White;

		if (iPass > ShaderPass::NonAnimPBR::Diffuse)
			iPass = ShaderPass::NonAnimPBR::White;

		//if (FAILED(m_pShaderCom->Begin(iPass)))
		//	return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::Diffuse)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(static_cast<_uint>(i))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEnvObject::Render_Shadow()
{
	if (!m_bRenderable || nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	size_t n = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < n; ++i)
	{
		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::Shadow)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render((_uint)i)))
			return E_FAIL;
	}
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
	if (nullptr == m_pTransformCom)
		return;

	m_LocalBounds.Transform(m_WorldBounds, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CEnvObject::Check_Visible()
{

	if (!m_bRenderable || !Has_RenderModel())
	{
		m_bVisible = false;
		m_bVisibleShadow = false;
		return;
	}

	const _bool bEnableShadow = ENABLE_ENV_OBJECT_SHADOW && m_bCastShadow;

	if (nullptr == m_pGameInstance_Proxy)
	{
		m_bVisible = true;
		m_bVisibleShadow = bEnableShadow;
		return;
	}

	m_bVisible = !m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::MAIN_CAMERA, m_WorldBounds);
	m_bVisibleShadow = bEnableShadow && !m_pGameInstance_Proxy->Should_CullAABB(CULLING_VIEW::SHADOW_DIR, m_WorldBounds);

	if ((m_bVisible || m_bVisibleShadow) && m_bEnableCulling)
	{
		const _bool bDistanceCulled =
			m_pGameInstance_Proxy->Should_CullByDistance(m_WorldBounds, ENV_DISTANCE_CULL_START);

		if (bDistanceCulled)
		{
			m_bVisible = false;
			m_bVisibleShadow = false;
		}
	}
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

	Refresh_WorldBounds();
}

void CEnvObject::Apply_DescDefaults()
{
	m_bRenderable = !m_tDesc.tCollision.bInvisibleCollision;
	m_bEnableCulling = m_tDesc.tRender.bUseLodCulling;
	m_bCastShadow = m_tDesc.tRender.bShadowMappingCaster;
	m_bVisible = true;
}

void CEnvObject::Free()
{
	Release_PhysicsActor();

	__super::Free();
}

NS_END
