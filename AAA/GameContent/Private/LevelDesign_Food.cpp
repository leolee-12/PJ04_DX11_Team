#include "LevelDesign_Food.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "Kirby.h"

#include "GameInstance.h"

namespace
{
	constexpr _float s_fFoodFloatHeight = 0.25f;
	constexpr _float s_fFoodRotationPerSec = 360.f;

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

	m_tFoodDesc = *static_cast<const LD_FOOD_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_RotationPerSec(s_fFoodRotationPerSec);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorAdd(m_pTransformCom->Get_State(STATE::POSITION), XMVectorSet(0.f, s_fFoodFloatHeight, 0.f, 0.f)));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_CullBounds_RotationInvariant(m_pModelCom)))
		return E_FAIL;

	m_bUseShadow = true;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Food::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tFoodDesc.eCategory != LD_CATEGORY::FOOD)
		return E_FAIL;
	if (m_tFoodDesc.wstrModelProtoTag.empty())
		return E_FAIL;
	if (m_tFoodDesc.fHealAmount < 0.f)
		return E_FAIL;

	const LD_FOOD_CATALOG* pCatalog = Find_FoodCatalog(m_tFoodDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;
	if (nullptr == pCatalog->pModelProtoTag || m_tFoodDesc.wstrModelProtoTag != pCatalog->pModelProtoTag)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || nullptr == m_pHurtBox)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Food::Late_Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	m_pTransformCom->Turn(XMVector3Normalize(XMVectorSet(0.f, 1.f, 0.f, 0.f)), fTimeDelta);

	if (m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}

	Check_Visible();
	Submit_RenderGroups();
}

HRESULT CLevelDesign_Food::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

HRESULT CLevelDesign_Food::Render_Shadow()
{
	return Render_ShadowModel(m_pShaderCom, m_pModelCom, MESH_LAYER_PROFILE::WORLD_NONANIM);
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

HRESULT CLevelDesign_Food::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	SetUp_Collider_Callback();

	return S_OK;
}

HRESULT CLevelDesign_Food::Ready_RenderComponents()
{
	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
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

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Food::Render_Model()
{
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
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::DMN);

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

const _tchar* CLevelDesign_Food::Resolve_ModelProtoTag() const
{
	return m_tFoodDesc.wstrModelProtoTag.c_str();
}

HRESULT CLevelDesign_Food::Ready_HurtBox()
{
	_float3 vMin = {};
	_float3 vMax = {};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	const _float3 vHalfExtents = { (vMax.x - vMin.x) * 0.5f, (vMax.y - vMin.y) * 0.5f, (vMax.z - vMin.z) * 0.5f };
	const _float fBoundsRadius = XMVectorGetX(XMVector3Length(XMVectorSet(vHalfExtents.x, vHalfExtents.y, vHalfExtents.z, 0.f)));

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.fRadius = fBoundsRadius;

	m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"),
		&ColliderDesc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLevelDesign_Food::SetUp_Collider_Callback()
{
	if (nullptr == m_pHurtBox)
		return;

	m_pHurtBox->Set_OnEnter([this](CCollider* pOther) { Handle_Pickup(pOther); });
}

void CLevelDesign_Food::Handle_Pickup(CCollider* pOther)
{
	if (nullptr == pOther)
		return;
	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return;
	if (Is_Dead())
		return;

	CKirby* pKirby = dynamic_cast<CKirby*>(pOther->Get_Owner());
	if (nullptr == pKirby)
		return;

	pKirby->Add_HP(m_tFoodDesc.fHealAmount);

	if (m_pHurtBox)
		m_pHurtBox->Set_Enabled(false);

	Set_Active(false);
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
