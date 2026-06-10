#include "EnvObject.h"
#include "GameContent_const.h"

#include "GameInstance.h"

#include <cmath>

NS_BEGIN(Client)

namespace
{
	constexpr _bool ENABLE_ENV_OBJECT_SHADOW = false;
	constexpr _float ENV_DISTANCE_CULL_START = 175.f;

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
	//, m_bRenderable{ true }
	//, m_bEnableCulling{ true }
	//, m_bCastShadow{ true }
{
}

CEnvObject::CEnvObject(const CEnvObject& Prototype)
	: CGameObject(Prototype)
	, m_tDesc(Prototype.m_tDesc)
	, m_strProtoTag(Prototype.m_strProtoTag)
	//, m_bRenderable{ Prototype.m_bRenderable }
	//, m_bEnableCulling{ Prototype.m_bEnableCulling }
	//, m_bCastShadow{ Prototype.m_bCastShadow }
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
	__super::Free();
}

NS_END
