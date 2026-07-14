#include "MapObject.h"
#include "MeshLayer_Binder.h"
#include "GameInstance.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CMapObject::Initialize_Prototype()
{
	m_iMaterialID = WORLD_STATIC_ID;
	return S_OK;
}

HRESULT CMapObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_MapComponents()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Validate_Initialized()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

void CMapObject::Late_Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CMapObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint n = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < n; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		if (FAILED(Render_MapMesh(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapObject::Bind_WorldMatrix()
{
	return m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
}

HRESULT CMapObject::Ready_MapComponents()
{
	m_pShaderCom = Add_Component<CShader>(Shader_MapEx.iLevelID, Shader_MapEx.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(Get_ModelProtoLevel(), Get_ModelProtoTag(), TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Bind_ShaderResources()
{
	if (FAILED(Bind_WorldMatrix()))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Bind_ShadowTransforms()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Render_MapMesh(_uint iMesh, const _float4x4* pWorldOverride)
{
	if (nullptr != pWorldOverride)
	{
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", pWorldOverride)))
			return E_FAIL;
	}

	const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(iMesh);

	MESH_LAYER_BIND_CONTEXT Ctx{};
	Ctx.pShader = m_pShaderCom;
	Ctx.pModel = m_pModelCom;
	Ctx.pGI_Proxy = m_pGameInstance_Proxy;
	Ctx.iMesh = iMesh;
	Ctx.pLayer = &Layer;
	Ctx.eProfile = MESH_LAYER_PROFILE::MAP;
	Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
	Ctx.iFallbackPass = ETOI(MAP_DEFAULT_PASS);
	Ctx.bUseLayerEx = true;

	MESH_LAYER_BIND_RESULT Result{};
	if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
		return E_FAIL;

	if (Result.bSkipMesh)
		return S_OK;

	if (FAILED(m_pShaderCom->Begin(Result.iPass)))
		return E_FAIL;
	if (FAILED(m_pModelCom->Render(iMesh)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Render_ShadowMesh(_uint iMesh, const _float4x4* pWorldOverride)
{
	if (nullptr != pWorldOverride)
	{
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", pWorldOverride)))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Begin(ETOI(MAP_PASS::SHADOW))))
		return E_FAIL;
	if (FAILED(m_pModelCom->Render(iMesh)))
		return E_FAIL;

	return S_OK;
}

void CMapObject::Free()
{
	__super::Free();
}
