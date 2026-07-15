#include "LevelDesign_Point.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "MeshLayer_Binder.h"
#include "GameInstance.h"

namespace
{
	constexpr const _tchar* POINTSTAR_PICKUP_SOUND = L"ItemPointStar_YellowCatched.wav";
	constexpr _float s_fPointRotationPerSec = 360.f;
	constexpr _float s_fPointPickupDuration = 0.75f;
	constexpr _float s_fPointPickupHeight = 3.f;
	constexpr _float s_fPointPickupTurnCount = 3.f;

	struct LD_POINT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		_int iValue;
		_bool bRotate;
	};

	static const LD_POINT_CATALOG g_PointCatalog[] =
	{
		{ L"PointStarYellow", CLevelDesign_Point::YELLOW_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopYellowL.ysh", 1, true },
		{ L"PointStarGreen", CLevelDesign_Point::GREEN_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopGreenL.ysh", 5, true },
		{ L"PointStarRed", CLevelDesign_Point::RED_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopRedL.ysh", 10, true },
		{ L"PointStarBlue", CLevelDesign_Point::BLUE_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopBlueL.ysh", 30, true },
		{ L"CoinClusterS", CLevelDesign_Point::COIN_CLUSTER_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterSL.ysh", 100, false },
		{ L"CoinClusterM", CLevelDesign_Point::COIN_CLUSTER_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterML.ysh", 250, false },
		{ L"CoinClusterL", CLevelDesign_Point::COIN_CLUSTER_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterLL.ysh", 500, false }
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
	, m_bRotate(Prototype.m_bRotate)
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

	m_tPointDesc = *static_cast<const LD_POINT_DESC*>(pArg);

	const LD_POINT_CATALOG* pCatalog = Find_PointCatalog(m_tPointDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;

	m_bRotate = pCatalog->bRotate;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (m_bRotate)
		m_pTransformCom->Set_RotationPerSec(s_fPointRotationPerSec);

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_CullingState(m_pModelCom, 0.f, true)))
		return E_FAIL;

	m_bUseShadow = true;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Point::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tPointDesc.eCategory != LD_CATEGORY::ITEM)
		return E_FAIL;
	if (m_tPointDesc.wstrModelProtoTag.empty())
		return E_FAIL;
	if (m_tPointDesc.iValue <= 0)
		return E_FAIL;

	const LD_POINT_CATALOG* pCatalog = Find_PointCatalog(m_tPointDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;
	if (nullptr == pCatalog->pModelProtoTag || m_tPointDesc.wstrModelProtoTag != pCatalog->pModelProtoTag)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || nullptr == m_pHurtBox)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Point::Late_Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	if (m_bPickingUp)
	{
		Update_Pickup(fTimeDelta);

		if (!m_bActive)
			return;
	}
	else if (m_bRotate)
	{
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
	Submit_RenderGroups();
}

HRESULT CLevelDesign_Point::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Model();
}

HRESULT CLevelDesign_Point::Render_Shadow()
{
	return Render_ShadowModel(m_pShaderCom, m_pModelCom, MESH_LAYER_PROFILE::WORLD_NONANIM);
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

HRESULT CLevelDesign_Point::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	SetUp_Collider_Callback();

	return S_OK;
}

HRESULT CLevelDesign_Point::Ready_RenderComponents()
{
	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
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

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Point::Render_Model()
{
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);
		const _bool bUseColorPass = (0u == m_pModelCom->Get_MeshTextureCount(i, MTEX_TYPE::DIFFUSE));

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.pShader = m_pShaderCom;
		Ctx.pModel = m_pModelCom;
		Ctx.pCullingState = m_pCullingState;
		Ctx.pGI_Proxy = m_pGameInstance_Proxy;
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = bUseColorPass ? ETOUI(WORLD_PASS::COLOR_CONST_MRA) : ETOUI(WORLD_PASS::DMN);

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
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

HRESULT CLevelDesign_Point::Ready_HurtBox()
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

void CLevelDesign_Point::SetUp_Collider_Callback()
{
	if (nullptr == m_pHurtBox)
		return;

	m_pHurtBox->Set_OnEnter([this](CCollider* pOther) { Handle_Pickup(pOther); });
}

void CLevelDesign_Point::Handle_Pickup(CCollider* pOther)
{
	if (nullptr == pOther)
		return;
	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return;
	if (Is_Dead() || m_bPickingUp)
		return;

	CGameObject* pPlayer = pOther->Get_Owner();
	if (nullptr == pPlayer)
		return;

	_float3 vStartPosition{};
	XMStoreFloat3(&vStartPosition, pPlayer->Get_Transform()->Get_State(STATE::POSITION));

	KIRBY_POINTSTAR_GAINED_DESC Desc{};
	Desc.iAmount = static_cast<_uint>(m_tPointDesc.iValue);
	m_pGameInstance_Proxy->Publish(EventTag::Kirby_PointStarGained, &Desc);

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	m_pGameInstance_Proxy->Play_SFX3D(POINTSTAR_PICKUP_SOUND, vPos, 0.5f);

	if (m_pHurtBox)
		m_pHurtBox->Set_Enabled(false);

	if (!m_bRotate)
	{
		Set_Active(false);
		return;
	}

	Begin_Pickup(vStartPosition);
}

void CLevelDesign_Point::Begin_Pickup(const _float3& vPickupStartPos)
{
	const _float3 vScale = m_pTransformCom->Get_Scaled();
	const _vector vStartPos = XMVectorSetW(XMLoadFloat3(&vPickupStartPos), 1.f);
	const _vector vTargetPos = vStartPos + XMVectorSet(0.f, s_fPointPickupHeight, 0.f, 0.f);

	XMStoreFloat3(&m_vPickupStartPos, vStartPos);
	XMStoreFloat3(&m_vPickupTargetPos, vTargetPos);

	m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(vScale.x, 0.f, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::UP, XMVectorSet(0.f, vScale.y, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, vScale.z, 0.f));
	m_pTransformCom->Set_State(STATE::POSITION, vStartPos);

	m_fPickupElapsed = 0.f;
	m_bPickingUp = true;
}

void CLevelDesign_Point::Update_Pickup(_float fTimeDelta)
{
	m_fPickupElapsed = min(m_fPickupElapsed + fTimeDelta, s_fPointPickupDuration);

	const _float fRatio = m_fPickupElapsed / s_fPointPickupDuration;
	const _float fEaseRatio = 1.f - powf(1.f - fRatio, 3.f);
	const _float fRotationRadian = XM_2PI * s_fPointPickupTurnCount * fEaseRatio;

	const _vector vStartPos = XMVectorSetW(XMLoadFloat3(&m_vPickupStartPos), 1.f);
	const _vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vPickupTargetPos), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorLerp(vStartPos, vTargetPos, fEaseRatio));
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), fRotationRadian);

	if (fRatio >= 1.f)
		Set_Active(false);
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
