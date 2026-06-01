#include "EnvObject.h"

#include "CullingContext.h"
#include "GameContent_const.h"
#include "GameInstance_Proxy.h"
#include "Model.h"
#include "Shader.h"

NS_BEGIN(Client)

namespace
{
	_matrix Build_WorldMatrix_FromTRS(const ENV_OBJECT_DESC& Desc)
	{
		const _vector vScale = XMLoadFloat3(&Desc.vScale);
		const _vector vRotation = XMLoadFloat4(&Desc.vRotation);
		const _vector vPosition = XMVectorSet(Desc.vPosition.x, Desc.vPosition.y, Desc.vPosition.z, 1.f);

		return XMMatrixScalingFromVector(vScale)
			* XMMatrixRotationQuaternion(vRotation)
			* XMMatrixTranslationFromVector(vPosition);
	}
}

CEnvObject::CEnvObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
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

HRESULT CEnvObject::Render_Model()
{
	if (nullptr == m_pModelCom)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", static_cast<_uint>(i), MTEX_TYPE::DIFFUSE, 0)))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", static_cast<_uint>(i), MTEX_TYPE::NORMALS, 0)))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", static_cast<_uint>(i), MTEX_TYPE::METALNESS, 0)))
			continue;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnkownTexture", static_cast<_uint>(i), MTEX_TYPE::UNKNOWN, 0)))
			continue;

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(static_cast<_uint>(i))))
			return E_FAIL;
	}

	return S_OK;
}

void CEnvObject::Update_LocalBounds()
{
	if (nullptr == m_pModelCom)
	{
		m_LocalBounds = ::MapTool::CCullingContext::Make_DefaultAABB();
		return;
	}

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (!::MapTool::CCullingContext::Is_ValidAABB(vMin, vMax))
	{
		m_LocalBounds = ::MapTool::CCullingContext::Make_DefaultAABB();
		return;
	}

	m_LocalBounds = ::MapTool::CCullingContext::Make_AABB_FromMinMax(vMin, vMax);
}

void CEnvObject::Refresh_WorldBounds()
{
	if (nullptr == m_pTransformCom)
		return;

	::MapTool::CCullingContext::Transform_AABB(
		m_LocalBounds,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()),
		&m_WorldBounds);
}

_bool CEnvObject::Is_VisibleInCurrentView() const
{
	if (!m_bEnableCulling || nullptr == m_pGameInstance_Proxy)
		return true;

	::MapTool::CCullingContext* pContext = ::MapTool::CCullingContext::Create();
	if (nullptr == pContext)
		return true;

	pContext->Update_FromViewProj(
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType),
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType));

	const _bool bVisible = !pContext->Should_CullAABB(true, m_WorldBounds);
	Safe_Release(pContext);
	return bVisible;
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
	m_bVisible = true;
}

void CEnvObject::Free()
{
	__super::Free();
}

NS_END
