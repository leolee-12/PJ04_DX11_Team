#include "LevelDesign_Food.h"
#include "LevelDesign_Registry.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	struct LD_FOOD_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		_float fHealAmount;
	};

	static const LD_FOOD_CATALOG g_FoodCatalog[] =
	{
		{ L"EnergyDrink", CLevelDesign_Food::ENERGY_DRINK_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Food/EnergyDrink.ysh", 10.f },
		{ L"DinnerRoastChicken", CLevelDesign_Food::DINNER_ROAST_CHICKEN_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Food/RoastChicken.ysh", 10.f },
		{ L"FruitCherry", CLevelDesign_Food::FRUIT_CHERRY_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Food/Cherry.ysh", 10.f },
		{ L"VegetableCarrot", CLevelDesign_Food::VEGETABLE_CARROT_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Food/Carrot.ysh", 10.f },
		{ L"SweetsDoughnut", CLevelDesign_Food::SWEETS_DOUGHNUT_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Food/Doughnut.ysh", 10.f },
		{ L"FruitBanana", CLevelDesign_Food::FRUIT_BANANA_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Food/Banana.ysh", 10.f }
	};

	static const LD_FOOD_CATALOG* Find_FoodCatalog(const _wstring& wstrObjName)
	{
		for (const LD_FOOD_CATALOG& Entry : g_FoodCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}
}

NS_BEGIN(Client)

CLevelDesign_Food::CLevelDesign_Food(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Food::CLevelDesign_Food(const CLevelDesign_Food& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tFoodDesc(Prototype.m_tFoodDesc)
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

void CLevelDesign_Food::Register_LevelDesignSpecs()
{
	for (const LD_FOOD_CATALOG& Entry : g_FoodCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Item";
		Spec.eCategory = LD_CATEGORY::FOOD;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
				{ Entry.pModelProtoTag, Entry.pModelPath, MODEL::NONANIM }
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_Food::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;
	if (Spec.wstrModelProtoTag.empty())
		return false;

	const LD_FOOD_CATALOG* pCatalog = Find_FoodCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog)
		return false;

	LD_FOOD_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = LD_CATEGORY::FOOD;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.fHealAmount = pCatalog->fHealAmount;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_Food::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Food::Create(pDevice, pContext);
}

HRESULT CLevelDesign_Food::Validate_Desc()
{
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

	m_pModelCom = Add_Component<CModel>(m_tFoodDesc.iModelProtoLevel, pModelProtoTag, TEXT("Com_Model"));
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