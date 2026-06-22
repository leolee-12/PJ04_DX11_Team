#include "LevelDesign_Food.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CLevelDesign_Food::CLevelDesign_Food(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Food::CLevelDesign_Food(const CLevelDesign_Food& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tFoodDesc(Prototype.m_tFoodDesc)
	, m_iModelProtoLevel(Prototype.m_iModelProtoLevel)
{
}

HRESULT CLevelDesign_Food::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Food::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tFoodDesc = *static_cast<const LD_FOOD_DESC*>(pArg);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Food::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Food::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLevelDesign_Food::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

LD_FOOD_TYPE CLevelDesign_Food::Resolve_FoodType(const _wstring& wstrObjName)
{
	static const pair<const _tchar*, LD_FOOD_TYPE> Catalog[] =
	{
		{ L"EnergyDrink",			LD_FOOD_TYPE::ENERGY_DRINK },
		{ L"DinnerRoastChicken",	LD_FOOD_TYPE::DINNER_ROAST_CHICKEN },
		{ L"FruitCherry",			LD_FOOD_TYPE::FRUIT_CHERRY },
		{ L"VegetableCarrot",		LD_FOOD_TYPE::VEGETABLE_CARROT },
		{ L"SweetsDoughnut",		LD_FOOD_TYPE::SWEETS_DOUGHNUT },
		{ L"FruitBanana",			LD_FOOD_TYPE::FRUIT_BANANA }
	};

	for (const auto& [pName, eType] : Catalog)
	{
		if (JsonUtils::Equals_NoCase(pName, wstrObjName.c_str()))
			return eType;
	}

	return LD_FOOD_TYPE::UNKNOWN;
}

HRESULT CLevelDesign_Food::Validate_Desc()
{
	if (LD_FOOD_TYPE::UNKNOWN == m_tFoodDesc.eType)
		return E_FAIL;
	if (m_tFoodDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Food::Ready_Components()
{
	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(
		Shader_NonAnimMesh_PBR.iLevelID,
		Shader_NonAnimMesh_PBR.szProtoTag,
		TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iModelProtoLevel, pModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Food::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Food::Render_Model()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
			{
				const _uint iLayerIndex = Layer.idx[ETOUI(eType)];
				const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(i, eType);

				if (0u < iTextureCount)
				{
					const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

					if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pConstantName, i, eType, iSafeIndex)))
						return S_OK;
				}

				return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
			};

		if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))             return E_FAIL;
		if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))			return E_FAIL;
		if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))                   return E_FAIL;
		if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))               return E_FAIL;

		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint))))                        return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint))))					return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float))))                     return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

const _tchar* CLevelDesign_Food::Resolve_ModelProtoTag() const
{
	return m_tFoodDesc.wstrModelProtoTag.c_str();
}


CLevelDesign_Food* CLevelDesign_Food::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Food* pInstance = new CLevelDesign_Food(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Food");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Food::Clone(void* pArg)
{
	CLevelDesign_Food* pInstance = new CLevelDesign_Food(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Food");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Food::Free()
{
	__super::Free();
}

NS_END