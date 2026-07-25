#include "LD_BattleBoundary.h"
#include "LevelDesign_Registry.h"
#include "World_BlendCollector.h"
#include "GameContent_Events.h"
#include "Parsing_Utils.h"

#include "Model.h"
#include "GameInstance.h"

NS_BEGIN(Client)

namespace
{
	constexpr _float BATTLE_BOUNDARY_FADE_IN_DURATION = 0.8f;
	constexpr _float BATTLE_BOUNDARY_FADE_OUT_DURATION = 1.2f;

	struct BATTLE_BOUNDARY_VARIANT
	{
		const _tchar* pObjectName;
		const _tchar* pPrototypeTag;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
	};

	constexpr BATTLE_BOUNDARY_VARIANT BATTLE_BOUNDARY_VARIANTS[] =
	{
		{ CLD_BattleBoundary::OBJECT_NAME, CLD_BattleBoundary::PROTOTYPE_TAG,
		  CLD_BattleBoundary::MODEL_PROTO_TAG, CLD_BattleBoundary::MODEL_PATH },

		{ CLD_BattleBoundary::OBJECT_NAME_CYLINDRICAL, CLD_BattleBoundary::PROTOTYPE_TAG_CYLINDRICAL,
		  CLD_BattleBoundary::MODEL_PROTO_TAG_CYLINDRICAL, CLD_BattleBoundary::MODEL_PATH_CYLINDRICAL },
	};

	const BATTLE_BOUNDARY_VARIANT* Find_Variant_ByObjectName(const _wstring& strObjectName)
	{
		for (const BATTLE_BOUNDARY_VARIANT& Variant : BATTLE_BOUNDARY_VARIANTS)
		{
			if (JsonUtils::Equals_NoCase(Variant.pObjectName, strObjectName.c_str()))
				return &Variant;
		}

		return nullptr;
	}

	const BATTLE_BOUNDARY_VARIANT* Find_Variant_ByModelProtoTag(const _wstring& strModelProtoTag)
	{
		for (const BATTLE_BOUNDARY_VARIANT& Variant : BATTLE_BOUNDARY_VARIANTS)
		{
			if (strModelProtoTag == Variant.pModelProtoTag)
				return &Variant;
		}

		return nullptr;
	}
}

CLD_BattleBoundary::CLD_BattleBoundary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_BattleBoundary::CLD_BattleBoundary(const CLD_BattleBoundary& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tBattleBoundaryDesc(Prototype.m_tBattleBoundaryDesc)
{
}

HRESULT CLD_BattleBoundary::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_BattleBoundary::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tBattleBoundaryDesc = *static_cast<const LD_BATTLE_BOUNDARY_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	Cache_BlendMeshIndices();

	if (FAILED(Ready_CullingState(m_pModelCom)))
		return E_FAIL;

	if (FAILED(Ready_PhysicsActor()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_BattleBoundary::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (nullptr == Find_Variant_ByObjectName(m_tBattleBoundaryDesc.strObjectName))
		return E_FAIL;

	if (LD_CATEGORY::GIMMICK != m_tBattleBoundaryDesc.eCategory)
		return E_FAIL;

	if (nullptr == Find_Variant_ByModelProtoTag(m_tBattleBoundaryDesc.wstrModelProtoTag))
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom)
		return E_FAIL;

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_BattleBoundary::Ready_Events()
{
	if (FAILED(__super::Ready_Events()))
		return E_FAIL;

	// 미니보스/보스 공통. Boss_HP_Appeared는 인트로 종료 시점에 발행된다(BossBase.cpp:94).
	Subscribe_Event(EventTag::Boss_HP_Appeared, [this](void*) { m_fAlphaTarget = 1.f; });
	Subscribe_Event(EventTag::Boss_Died, [this](void*) { m_fAlphaTarget = 0.f; });

	return S_OK;
}

void CLD_BattleBoundary::Update(_float fTimeDelta)
{
	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		m_fAlpha = 1.f;	// 에디터에서는 항상 보여야 배치 가능
		return;
	}

	const _bool bFadeIn = m_fAlphaTarget > m_fAlpha;
	const _float fDuration = bFadeIn ? BATTLE_BOUNDARY_FADE_IN_DURATION : BATTLE_BOUNDARY_FADE_OUT_DURATION;
	const _float fStep = fDuration > 0.f ? fTimeDelta / fDuration : 1.f;

	m_fAlpha = bFadeIn ? min(m_fAlphaTarget, m_fAlpha + fStep) : max(m_fAlphaTarget, m_fAlpha - fStep);

	if (m_fAlpha != m_fAlphaTarget)	// 등장/퇴장 중 = 위치가 옮겨지는 구간
		Sync_PhysicsActor();
}

void CLD_BattleBoundary::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead() || m_fAlpha <= 0.001f)
		return;

	Check_Visible();
	Submit_RenderGroups();		// 불투명 패스로 저작된 메쉬 대비
	Submit_BlendMeshes();
}

HRESULT CLD_BattleBoundary::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Render_Mesh(i, MESH_LAYER_RENDER_KIND::MAIN)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLD_BattleBoundary::Render_BlendMesh(_uint iMeshIndex)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Mesh(iMeshIndex, MESH_LAYER_RENDER_KIND::MAIN_BLEND);
}

HRESULT CLD_BattleBoundary::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (FAILED(__super::Apply_EditMeshLayer(iModelSlot, iMesh, Layer)))
		return E_FAIL;

	Cache_BlendMeshIndices();		// 패스를 블렌드로 바꾸면 제출 목록도 갱신돼야 한다
	return S_OK;
}

void CLD_BattleBoundary::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	const BATTLE_BOUNDARY_VARIANT* pVariant = Find_Variant_ByModelProtoTag(m_tBattleBoundaryDesc.wstrModelProtoTag);

	pOutData->strPrototypeTag = nullptr != pVariant ? pVariant->pPrototypeTag : PROTOTYPE_TAG;
}

void CLD_BattleBoundary::Deserialize_Internal(const json& j)
{
	__super::Deserialize_Internal(j);

	// AddedMapObjects 는 스폰이 끝난 뒤 트랜스폼이 들어온다(Map_Spawner.cpp:488).
	// Create_StaticActor 가 스케일을 PxMeshScale 로 구워버리므로 여기서 다시 만든다.
	Rebuild_PhysicsActor();
}

HRESULT CLD_BattleBoundary::On_EditTransformChanged()
{
	if (FAILED(__super::On_EditTransformChanged()))
		return E_FAIL;

	Rebuild_PhysicsActor();
	return S_OK;
}

void CLD_BattleBoundary::Register_LevelDesignSpecs()
{
	for (const BATTLE_BOUNDARY_VARIANT& Variant : BATTLE_BOUNDARY_VARIANTS)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Variant.pObjectName;
		Spec.strPrototypeTag = Variant.pPrototypeTag;
		Spec.strLayerTag = LAYER_TAG;
		Spec.eCategory = LD_CATEGORY::GIMMICK;
		Spec.wstrModelProtoTag = Variant.pModelProtoTag;
		Spec.eModelType = MODEL::NONANIM;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.pMakeDefaultDesc = &Make_DefaultDesc;		// 없으면 MapTool 팔레트에 안 뜬다
		Spec.ModelRequirements =
		{
			{ Variant.pModelProtoTag, Variant.pModelPath, MODEL::NONANIM, true }
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLD_BattleBoundary::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	const BATTLE_BOUNDARY_VARIANT* pVariant = Find_Variant_ByObjectName(CommonDesc.strObjectName);
	if (nullptr == pVariant)
		return false;

	if (!JsonUtils::Equals_NoCase(Spec.strObjectName.c_str(), CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != pVariant->pPrototypeTag || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (LD_CATEGORY::GIMMICK != Spec.eCategory || MODEL::NONANIM != Spec.eModelType
		|| Spec.wstrModelProtoTag != pVariant->pModelProtoTag)
		return false;

	LD_BATTLE_BOUNDARY_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	*pOutEntry = std::move(Desc);
	return true;
}

_bool CLD_BattleBoundary::Make_DefaultDesc(const LD_OBJECT_DESC& CommonDesc, _uint iModelProtoLevel,
	const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	if (!Build_Desc(CommonDesc, json::object(), Spec, pOutEntry))
		return false;

	LD_BATTLE_BOUNDARY_DESC* pDesc = get_if<LD_BATTLE_BOUNDARY_DESC>(pOutEntry);
	if (nullptr == pDesc)
		return false;

	pDesc->iModelProtoLevel = iModelProtoLevel;
	return true;
}

CGameObject* CLD_BattleBoundary::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_BattleBoundary::Create(pDevice, pContext);
}

HRESULT CLD_BattleBoundary::Ready_RenderComponents()
{
	if (m_tBattleBoundaryDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tBattleBoundaryDesc.iModelProtoLevel,
		m_tBattleBoundaryDesc.wstrModelProtoTag.c_str(), TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_BattleBoundary::Bind_ShaderResources()
{
	if (FAILED(MeshLayerBinder::Bind_WorldViewProj(m_pShaderCom, m_pTransformCom, m_pGameInstance_Proxy, m_eProjType)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_BattleBoundary::Render_Mesh(_uint iMeshIndex, MESH_LAYER_RENDER_KIND eKind)
{
	const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(iMeshIndex);

	MESH_LAYER_BIND_CONTEXT Ctx{};
	Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
	Ctx.iMesh = iMeshIndex;
	Ctx.pLayer = &Layer;
	Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
	Ctx.eKind = eKind;
	Ctx.iFallbackPass = ETOUI(WORLD_PASS::BLEND_UKWN_BARRIER);

	_uint iPass = 0u;
	const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
	if (FAILED(hrBind))		return E_FAIL;
	if (S_FALSE == hrBind)	return S_OK;

	// Bind_WorldCommonParams가 Layer.vRenderColor로 g_vColor를 이미 바인딩했다(MeshLayer_Binder.cpp:311).
	// 알파만 곱해 덮어쓴다. CModel::Set_MeshLayer로 모델을 건드리면 _meshlayer.json이 오염된다.
	_float4 vRenderColor = Layer.vRenderColor;
	vRenderColor.w *= m_fAlpha;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &vRenderColor, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(iPass)))
		return E_FAIL;

	return m_pModelCom->Render(iMeshIndex);
}

void CLD_BattleBoundary::Cache_BlendMeshIndices()
{
	m_BlendMeshIndices.clear();

	if (nullptr == m_pModelCom)
		return;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (Is_WorldBlendPass(m_pModelCom->Get_MeshLayer(i).iPass))
			m_BlendMeshIndices.push_back(i);
	}
}

void CLD_BattleBoundary::Submit_BlendMeshes()
{
	if (!m_bVisible || m_BlendMeshIndices.empty() || nullptr == m_pModelCom)
		return;

	if (nullptr == m_pBlendCollector)
		m_pBlendCollector = CWorld_BlendCollector::Find(m_pGameInstance_Proxy);

	if (nullptr == m_pBlendCollector)
		return;

	const _float4x4* pWorld = m_pTransformCom->Get_WorldMatrixPtr();

	for (_uint iMeshIndex : m_BlendMeshIndices)
		m_pBlendCollector->Submit(this, this, m_pModelCom, pWorld, iMeshIndex);
}

HRESULT CLD_BattleBoundary::Ready_PhysicsActor()
{
	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(
		m_pModelCom->Get_CollisionMesh(), XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	return (nullptr != m_pRigidStatic) ? S_OK : E_FAIL;
}

void CLD_BattleBoundary::Rebuild_PhysicsActor()
{
	Release_PhysicsActor();
	Ready_PhysicsActor();
}

void CLD_BattleBoundary::Sync_PhysicsActor()
{
	m_pGameInstance_Proxy->Refresh_StaticActorPose(m_pRigidStatic, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CLD_BattleBoundary::Release_PhysicsActor()
{
	if (nullptr == m_pRigidStatic)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;
}

CLD_BattleBoundary* CLD_BattleBoundary::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_BattleBoundary* pInstance = new CLD_BattleBoundary(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_BattleBoundary");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_BattleBoundary::Clone(void* pArg)
{
	CLD_BattleBoundary* pInstance = new CLD_BattleBoundary(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_BattleBoundary");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_BattleBoundary::Free()
{
	Release_PhysicsActor();

	m_pBlendCollector = nullptr;	// Find()는 AddRef하지 않으므로 Release 없음
	m_BlendMeshIndices.clear();

	__super::Free();
}

NS_END