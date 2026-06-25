#include "LevelDesign_Point.h"
#include "LevelDesign_Registry.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	struct LD_POINT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		_int iValue;
	};

	static const LD_POINT_CATALOG g_PointCatalog[] =
	{
		{ L"PointStarYellow", CLevelDesign_Point::YELLOW_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopYellowL.ysh", 1 },
		{ L"PointStarGreen", CLevelDesign_Point::GREEN_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopGreenL.ysh", 5 },
		{ L"PointStarRed", CLevelDesign_Point::RED_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopRedL.ysh", 10 },
		{ L"PointStarBlue", CLevelDesign_Point::BLUE_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopBlueL.ysh", 30 },
		{ L"CoinClusterS", CLevelDesign_Point::COIN_CLUSTER_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterSL.ysh", 100 },
		{ L"CoinClusterM", CLevelDesign_Point::COIN_CLUSTER_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterML.ysh", 250 },
		{ L"CoinClusterL", CLevelDesign_Point::COIN_CLUSTER_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterLL.ysh", 500 }
	};

	static const LD_POINT_CATALOG* Find_PointCatalog(const _wstring& wstrObjName)
	{
		for (const LD_POINT_CATALOG& Entry : g_PointCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}
}

NS_BEGIN(Client)

CLevelDesign_Point::CLevelDesign_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Point::CLevelDesign_Point(const CLevelDesign_Point& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tPointDesc(Prototype.m_tPointDesc)
{
}

HRESULT CLevelDesign_Point::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Point::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tPointDesc = *static_cast<const LD_POINT_DESC*>(pArg);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Point::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Point::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

void CLevelDesign_Point::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLevelDesign_Point::Register_LevelDesignSpecs()
{
	for (const LD_POINT_CATALOG& Entry : g_PointCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Item";
		Spec.eCategory = LD_CATEGORY::ITEM;
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

_bool CLevelDesign_Point::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;
	if (Spec.wstrModelProtoTag.empty())
		return false;

	const LD_POINT_CATALOG* pCatalog = Find_PointCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog)
		return false;

	LD_POINT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = LD_CATEGORY::ITEM;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.iValue = pCatalog->iValue;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_Point::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Point::Create(pDevice, pContext);
}

HRESULT CLevelDesign_Point::Validate_Desc()
{
	if (m_tPointDesc.wstrModelProtoTag.empty())
		return E_FAIL;
	if (0 >= m_tPointDesc.iValue)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Point::Ready_Components()
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

	m_pModelCom = Add_Component<CModel>(m_tPointDesc.iModelProtoLevel, pModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Point::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Point::Render_Model()
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

const _tchar* CLevelDesign_Point::Resolve_ModelProtoTag() const
{
	if (m_tPointDesc.wstrModelProtoTag.empty())
		return nullptr;

	return m_tPointDesc.wstrModelProtoTag.c_str();
}

CLevelDesign_Point* CLevelDesign_Point::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Point* pInstance = new CLevelDesign_Point(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Point");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Point::Clone(void* pArg)
{
	CLevelDesign_Point* pInstance = new CLevelDesign_Point(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Point");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Point::Free()
{
	__super::Free();
}

NS_END