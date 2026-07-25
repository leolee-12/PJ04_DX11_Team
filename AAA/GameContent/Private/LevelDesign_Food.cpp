#include "LevelDesign_Food.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "Effect_Loader.h"
#include "Kirby.h"

#include "GameInstance.h"

namespace
{
	constexpr _bool s_bMoveFoodToPlayerOnPickup = false;

	constexpr _float s_fFoodFloatHeight = 0.25f;
	constexpr _float s_fFoodRotationPerSec = 360.f;
	constexpr _float s_fFoodPickupDuration = 0.75f;
	constexpr _float s_fFoodPickupHeight = 3.f;
	constexpr _float s_fFoodPickupTurnCount = 1.f;
	constexpr _float3 s_vPickupEffectOffset = { 0.f, 2.f, 0.f };

	constexpr _float s_fInhalePullAccel = 40.f;
	constexpr _float s_fInhaleMouthFwd = 0.6f;
	constexpr _float s_fInhaleMouthUp = 0.6f;
	constexpr _float s_fInhaleActivationGraceTime = 0.25f;

	constexpr _float s_fHealAmountLv1 = 10.f;
	constexpr _float s_fHealAmountLv2 = 20.f;
	constexpr _float s_fHealAmountLv3 = 30.f;


	constexpr const _tchar* ITEM_EFFECT_ID = L"ItemEffect";

	struct LD_FOOD_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		_float fHealAmount;
	};

	static const LD_FOOD_CATALOG g_FoodCatalog[] =
	{
		{ L"EnergyDrink", L"Proto_Component_Model_Food_EnergyDrink", "../../Resources/Map/Gimmick/NonAnim/Food/EnergyDrink.ysh", s_fHealAmountLv3 },
		{ L"DinnerRoastChicken", L"Proto_Component_Model_Food_RoastChicken", "../../Resources/Map/Gimmick/NonAnim/Food/RoastChicken.ysh", s_fHealAmountLv3 },
		{ L"FruitCherry", L"Proto_Component_Model_Food_Cherry", "../../Resources/Map/Gimmick/NonAnim/Food/Cherry.ysh", s_fHealAmountLv1 },
		{ L"VegetableCarrot", L"Proto_Component_Model_Food_Carrot", "../../Resources/Map/Gimmick/NonAnim/Food/Carrot.ysh", s_fHealAmountLv1 },
		{ L"SweetsDoughnut", L"Proto_Component_Model_Food_Doughnut", "../../Resources/Map/Gimmick/NonAnim/Food/Doughnut.ysh", s_fHealAmountLv2 },
		{ L"FruitBanana", L"Proto_Component_Model_Food_Banana", "../../Resources/Map/Gimmick/NonAnim/Food/Banana.ysh", s_fHealAmountLv1 },
		{ L"VegetablePumpkin", L"Proto_Component_Model_Food_Pumpkin", "../../Resources/Map/Gimmick/NonAnim/Food/Pumpkin.ysh", s_fHealAmountLv1 },
		{ L"DinnerSushi", L"Proto_Component_Model_Food_Sushi", "../../Resources/Map/Gimmick/NonAnim/Food/Sushi.ysh", s_fHealAmountLv3 },
		{ L"FruitMelon", L"Proto_Component_Model_Food_Melon", "../../Resources/Map/Gimmick/NonAnim/Food/Melon.ysh", s_fHealAmountLv2 },
		{ L"LightFriedegg", L"Proto_Component_Model_Food_Friedegg", "../../Resources/Map/Gimmick/NonAnim/Food/Friedegg.ysh", s_fHealAmountLv2 },
		{ L"DinnerSteak", L"Proto_Component_Model_Food_Steak", "../../Resources/Map/Gimmick/NonAnim/Food/Steak.ysh", s_fHealAmountLv3 },
		{ L"VegetableGreenpepper", L"Proto_Component_Model_Food_Greenpepper", "../../Resources/Map/Gimmick/NonAnim/Food/Greenpepper.ysh", s_fHealAmountLv1 },
		{ L"SweetsIceCandy", L"Proto_Component_Model_Food_IceCandy", "../../Resources/Map/Gimmick/NonAnim/Food/IceCandy.ysh", s_fHealAmountLv2 },
		{ L"CupJuiceMall", L"Proto_Component_Model_Food_CupJuiceMall", "../../Resources/Map/Gimmick/NonAnim/Food/CupJuiceMall.ysh", s_fHealAmountLv2 },
		{ L"CupJuicePark", L"Proto_Component_Model_Food_CupJuicePark", "../../Resources/Map/Gimmick/NonAnim/Food/CupJuicePark.ysh", s_fHealAmountLv2 },
		{ L"SweetsSoftCream", L"Proto_Component_Model_Food_SoftCream", "../../Resources/Map/Gimmick/NonAnim/Food/SoftCream.ysh", s_fHealAmountLv3 },
		{ L"DinnerOnigiri", L"Proto_Component_Model_Food_Onigiri", "../../Resources/Map/Gimmick/NonAnim/Food/Onigiri.ysh", s_fHealAmountLv2 },
		{ L"JunkPopcorn", L"Proto_Component_Model_Food_Popcorn", "../../Resources/Map/Gimmick/NonAnim/Food/Popcorn.ysh", s_fHealAmountLv2 }
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
	m_ItemEffectHandle.Clear();

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_RotationPerSec(s_fFoodRotationPerSec);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorAdd(m_pTransformCom->Get_State(STATE::POSITION), XMVectorSet(0.f, s_fFoodFloatHeight, 0.f, 0.f)));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_CullingState(m_pModelCom, 0.f, true)))
		return E_FAIL;

	m_bUseShadow = true;

	if (!m_tFoodDesc.strReceiveEventTag.empty())
	{
		Set_Active(false);
		m_pHurtBox->Set_Enabled(false);
	}
	else if (FAILED(Ready_Effect()))
	{
		return E_FAIL;
	}

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

	m_fInhaleGraceTime = max(0.f, m_fInhaleGraceTime - fTimeDelta);

	if (m_bPickingUp)
	{
		Update_Pickup(fTimeDelta);

		if (!m_bActive)
			return;
	}
	else if (m_bInhalePullRequested)
	{
		Update_InhalePull(fTimeDelta);
		m_bInhalePullRequested = false;
	}
	else
	{
		m_fInhalePullSpeed = 0.f;
		m_pInhaler = nullptr;
		m_pTransformCom->Turn(XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP)), fTimeDelta);
	}

	if (m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}

	Check_Visible();
	m_bInhaleDisplaced = false;
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

void CLevelDesign_Food::On_LDEventReceived(const _wstring& strEventTag)
{
	UNREFERENCED_PARAMETER(strEventTag);

	if (m_bPickingUp || Is_Dead() || Is_Active())
		return;

	if (FAILED(Ready_Effect()))
		return;

	m_fInhaleGraceTime = s_fInhaleActivationGraceTime;
	m_bInhalePullRequested = false;
	m_fInhalePullSpeed = 0.f;
	m_pInhaler = nullptr;

	Set_Active(true);

	if (nullptr != m_pHurtBox)
	{
		m_pHurtBox->Set_Enabled(true);
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	}
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

HRESULT CLevelDesign_Food::Ready_Effect()
{
	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();
	if (pEffectLoader->Is_Current(m_ItemEffectHandle))
		return S_OK;

	m_ItemEffectHandle.Clear();

	_float3 vEffectPosition{};
	XMStoreFloat3(&vEffectPosition, m_pTransformCom->Get_State(STATE::POSITION));
	vEffectPosition.y += s_fFoodFloatHeight * 1.5f;

	if (FAILED(pEffectLoader->Spawn(
		ITEM_EFFECT_ID,
		Get_LevelIndex(),
		vEffectPosition,
		_float3(0.f, 0.f, 0.f),
		_float3(0.f, 0.f, 0.f),
		nullptr,
		nullptr,
		&m_ItemEffectHandle)))
	{
		m_ItemEffectHandle.Clear();
		return E_FAIL;
	}

	const _float3 vFoodScale = m_pTransformCom->Get_Scaled();
	m_ItemEffectHandle.p->Get_Transform()->Set_Scale(vFoodScale.x, vFoodScale.y, vFoodScale.z);

	return S_OK;
}

void CLevelDesign_Food::Release_Effect()
{
	if (nullptr == m_pGameInstance_Proxy || !m_pGameInstance_Proxy->IsConnected())
	{
		m_ItemEffectHandle.Clear();
		return;
	}

	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();
	if (pEffectLoader->Is_Current(m_ItemEffectHandle))
		m_ItemEffectHandle.p->EffectContainer_Stop();

	m_ItemEffectHandle.Clear();
}

HRESULT CLevelDesign_Food::Bind_ShaderResources()
{
	if (FAILED(MeshLayerBinder::Bind_WorldViewProj(m_pShaderCom, m_pTransformCom, m_pGameInstance_Proxy, m_eProjType)))
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
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::DMN);

		_uint iPass = 0u;
		const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
		if (FAILED(hrBind))     return E_FAIL;
		if (S_FALSE == hrBind)  continue;

		if (FAILED(m_pShaderCom->Begin(iPass)))
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

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::LD_ITEM));

	return S_OK;
}

void CLevelDesign_Food::SetUp_Collider_Callback()
{
	if (nullptr == m_pHurtBox)
		return;

	m_pHurtBox->Set_OnEnter([this](CCollider* pOther) { Handle_Pickup(pOther); Handle_InhalePull(pOther); });
	m_pHurtBox->Set_OnStay([this](CCollider* pOther) { Handle_InhalePull(pOther); });
}

void CLevelDesign_Food::Handle_Pickup(CCollider* pOther)
{
	if (nullptr == pOther)
		return;
	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return;
	if (Is_Dead() || m_bPickingUp)
		return;

	CKirby* pKirby = dynamic_cast<CKirby*>(pOther->Get_Owner());
	if (nullptr == pKirby)
		return;

	_float3 vStartPos{};
	XMStoreFloat3(&vStartPos, pKirby->Get_Transform()->Get_State(STATE::POSITION));

	pKirby->Add_HP(m_tFoodDesc.fHealAmount);

	Release_Effect();

	if (m_pHurtBox)
		m_pHurtBox->Set_Enabled(false);

	m_bInhalePullRequested = false;
	m_fInhalePullSpeed = 0.f;
	m_pInhaler = nullptr;

	if (s_bMoveFoodToPlayerOnPickup)
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vStartPos), 1.f));

	_float3 vEffectPosition{};
	XMStoreFloat3(&vEffectPosition, m_pTransformCom->Get_State(STATE::POSITION) + XMLoadFloat3(&s_vPickupEffectOffset));
	CEffect_Loader::GetInstance()->Spawn(TEXT("PickUpEffect"), m_iLevelIndex, vEffectPosition);

	Begin_Pickup();
}

void CLevelDesign_Food::Begin_Pickup()
{
	const _float3 vScale = m_pTransformCom->Get_Scaled();
	const _vector vStartPos = XMVectorSetW(m_pTransformCom->Get_State(STATE::POSITION), 1.f);
	const _vector vTargetPos = vStartPos + XMVectorSet(0.f, s_fFoodPickupHeight, 0.f, 0.f);

	XMStoreFloat3(&m_vPickupStartPos, vStartPos);
	XMStoreFloat3(&m_vPickupTargetPos, vTargetPos);

	m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(vScale.x, 0.f, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::UP, XMVectorSet(0.f, vScale.y, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, vScale.z, 0.f));

	m_fPickupElapsed = 0.f;
	m_bPickingUp = true;
}

void CLevelDesign_Food::Update_Pickup(_float fTimeDelta)
{
	m_fPickupElapsed = min(m_fPickupElapsed + fTimeDelta, s_fFoodPickupDuration);

	const _float fRatio = m_fPickupElapsed / s_fFoodPickupDuration;
	const _float fEaseRatio = 1.f - powf(1.f - fRatio, 3.f);
	const _float fRotationRadian = XM_2PI * s_fFoodPickupTurnCount * fEaseRatio;

	const _vector vStartPos = XMVectorSetW(XMLoadFloat3(&m_vPickupStartPos), 1.f);
	const _vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vPickupTargetPos), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorLerp(vStartPos, vTargetPos, fEaseRatio));
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), fRotationRadian);

	if (fRatio >= 1.f)
		Set_Active(false);
}

void CLevelDesign_Food::Handle_InhalePull(CCollider* pOther)
{
	if (nullptr == pOther)
		return;
	if (ETOUI(COLLISION_LAYER::PLAYER_INHALE) != pOther->Get_RegisteredGroup())
		return;
	if (!Is_Active() || Is_Dead() || m_bPickingUp || m_fInhaleGraceTime > 0.f)
		return;

	CGameObject* pInhaler = pOther->Get_Owner();
	if (nullptr == pInhaler)
		return;

	m_pInhaler = pInhaler;
	m_bInhalePullRequested = true;
}

void CLevelDesign_Food::Update_InhalePull(_float fTimeDelta)
{
	if (nullptr == m_pInhaler)
		return;

	CTransform* pInhalerTransform = m_pInhaler->Get_Transform();
	const _vector vMouthPosition = pInhalerTransform->Get_State(STATE::POSITION)
		+ pInhalerTransform->Get_State(STATE::LOOK) * s_fInhaleMouthFwd
		+ pInhalerTransform->Get_State(STATE::UP) * s_fInhaleMouthUp;

	const _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	const _vector vDirection = vMouthPosition - vPosition;
	const _float fDistance = XMVectorGetX(XMVector3Length(vDirection));
	if (fDistance <= FLT_EPSILON)
		return;

	m_fInhalePullSpeed += s_fInhalePullAccel * fTimeDelta;
	const _float fMoveDistance = min(m_fInhalePullSpeed * fTimeDelta, fDistance);
	m_pTransformCom->Set_State(STATE::POSITION, vPosition + XMVector3Normalize(vDirection) * fMoveDistance);

	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();
	if (pEffectLoader->Is_Current(m_ItemEffectHandle))
	{
		const _vector vEffectPosition = m_pTransformCom->Get_State(STATE::POSITION)
			+ XMVectorSet(0.f, s_fFoodFloatHeight * 1.5f, 0.f, 0.f);
		m_ItemEffectHandle.p->Get_Transform()->Set_State(STATE::POSITION, XMVectorSetW(vEffectPosition, 1.f));
	}
	else
	{
		m_ItemEffectHandle.Clear();
	}

	if (fMoveDistance > 0.f)
		m_bInhaleDisplaced = true;

	m_pTransformCom->Turn(XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP)), fTimeDelta);
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
	Release_Effect();

	__super::Free();
}

NS_END
