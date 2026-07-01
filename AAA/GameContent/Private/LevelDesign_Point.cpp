#include "LevelDesign_Point.h"
#include "LevelDesign_Registry.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	constexpr _float s_fPointRotationPerSec = 360.f;

	struct LD_POINT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		_int iValue;
		_float4 vRenderColor;
		_bool bRotate;
	};

	static const LD_POINT_CATALOG g_PointCatalog[] =
	{
		{ L"PointStarYellow", CLevelDesign_Point::YELLOW_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopYellowL.ysh", 1, { 1.f, 0.843f, 0.f, 1.f }, true },
		{ L"PointStarGreen", CLevelDesign_Point::GREEN_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopGreenL.ysh", 5, { 0.2f, 0.85f, 0.25f, 1.f }, true },
		{ L"PointStarRed", CLevelDesign_Point::RED_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopRedL.ysh", 10, { 1.f, 0.2f, 0.18f, 1.f }, true },
		{ L"PointStarBlue", CLevelDesign_Point::BLUE_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopBlueL.ysh", 30, { 0.2f, 0.42f, 1.f, 1.f }, true },
		{ L"CoinClusterS", CLevelDesign_Point::COIN_CLUSTER_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterSL.ysh", 100, { 1.f, 0.843f, 0.f, 1.f }, false },
		{ L"CoinClusterM", CLevelDesign_Point::COIN_CLUSTER_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterML.ysh", 250, { 1.f, 0.843f, 0.f, 1.f }, false },
		{ L"CoinClusterL", CLevelDesign_Point::COIN_CLUSTER_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterLL.ysh", 500, { 1.f, 0.843f, 0.f, 1.f }, false }
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
	, m_vRenderColor(Prototype.m_vRenderColor)
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

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tPointDesc = *static_cast<const LD_POINT_DESC*>(pArg);

	if (const LD_POINT_CATALOG* pCatalog = Find_PointCatalog(m_tPointDesc.strObjectName))
	{
		m_vRenderColor = pCatalog->vRenderColor;
		m_bRotate = pCatalog->bRotate;
	}

	if (m_bRotate)
		m_pTransformCom->Set_RotationPerSec(s_fPointRotationPerSec);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Point::Late_Update(_float fTimeDelta)
{
	if (!m_bActive || Is_Dead())
		return;

	if (m_bRotate)
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);

	if (m_pPickupCollider && m_pPickupCollider->Is_Enabled())
	{
		m_pPickupCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pPickupCollider);
#endif
	}

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

	if (FAILED(Ready_PickupCollider()))
		return E_FAIL;

	SetUp_Collider_Callback();

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

		const _bool bUseColorPass = (0u == m_pModelCom->Get_MeshTextureCount(i, MTEX_TYPE::DIFFUSE));

		if (!bUseColorPass)
			if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA))) return E_FAIL;
		if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL))) return E_FAIL;
		if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA))) return E_FAIL;

		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float)))) return E_FAIL;

		_uint iPass = ShaderPass::NonAnimPBR::DMN;
		if (bUseColorPass)
		{
			const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vRenderColor, sizeof(_float4)))) return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4)))) return E_FAIL;
			iPass = ShaderPass::NonAnimPBR::COLOR;
		}

		if (FAILED(m_pShaderCom->Begin(iPass)))
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

HRESULT CLevelDesign_Point::Ready_PickupCollider()
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
	m_pPickupCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_PickupCollider"),
		&ColliderDesc);
	if (nullptr == m_pPickupCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pPickupCollider, ETOUI(COLLISION_LAYER::ENV_TRIGGER));
	m_bPickupColliderRegistered = true;

	return S_OK;
}

void CLevelDesign_Point::SetUp_Collider_Callback()
{
	if (nullptr == m_pPickupCollider)
		return;

	m_pPickupCollider->Set_OnEnter([this](CCollider* pOther) { Handle_Pickup(pOther); });
}

void CLevelDesign_Point::Handle_Pickup(CCollider* pOther)
{
	if (nullptr == pOther)
		return;
	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return;
	if (Is_Dead())
		return;

	KIRBY_POINTSTAR_GAINED_DESC Desc{};
	Desc.iAmount = static_cast<_uint>(m_tPointDesc.iValue);
	m_pGameInstance_Proxy->Publish(EventTag::Kirby_PointStarGained, &Desc);

	if (m_pPickupCollider)
		m_pPickupCollider->Set_Enabled(false);

	Unregister_PickupCollider(false);
	Set_Active(false);
	m_pGameInstance_Proxy->Destroy_GameObject(this);
}

void CLevelDesign_Point::Unregister_PickupCollider(_bool bImmediate)
{
	if (nullptr == m_pPickupCollider || !m_bPickupColliderRegistered)
		return;

	const _uint iGroup = ETOUI(COLLISION_LAYER::ENV_TRIGGER);
	if (bImmediate)
		m_pGameInstance_Proxy->Immediate_Unregister(m_pPickupCollider, iGroup);
	else
		m_pGameInstance_Proxy->Request_Unregister(m_pPickupCollider, iGroup);

	m_bPickupColliderRegistered = false;
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
	Unregister_PickupCollider(true);

	__super::Free();
}

NS_END