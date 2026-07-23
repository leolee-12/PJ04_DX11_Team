#include "LevelDesign_Breakable.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "Effect_Loader.h"

#include "GameInstance.h"

namespace
{
	constexpr const _tchar*		WOOD_BOX_BREAK_SOUNDKEY			= L"GimmickBlock_BoxWoodBreak.wav";
	constexpr const _tchar *	PLASTIC_BOX_BREAK_SOUNDKEY		= L"GimmickBlock_BoxPlasticBreak.wav";
	constexpr const _tchar*		NORMAL_ROCK_BREAK_SOUNDKEY		= L"GimmickBreakable_NormalRockBreak.wav";
	constexpr const _tchar*		LARGE_ROCK_BREAK_SOUNDKEY		= L"GimmickBreakable_LargeRockBreak.wav";

	static constexpr _float		DESTROY_EFFECT_HEIGHT_RATIO = 0.55f;
	static constexpr _float		BREAKABLE_CULL_MARGIN = 1.f;

	static constexpr _uint		BREAK_MASK_LV1 = 0xFFFFFFFFu;
	constexpr _uint Hit_Bit(HIT_TYPE eType) { return 1u << static_cast<_uint>(eType); }
	static constexpr _uint		BREAK_MASK_LV2 = Hit_Bit(HIT_TYPE::BREAKERABLE_HIT);

	struct LD_BREAKABLE_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		_uint iBreakAnimIndex;
		const _char* pBaseMeshName;
		_bool bCookCollisionMesh;
		const _tchar* pDestroyEffectId;
		
		// 사운드 관련 정보
		const _tchar*	pBreakSoundKey;
		_float			fVolume;

		_uint	iBreakHitMask;
	};

	static const LD_BREAKABLE_CATALOG g_BreakableCatalog[] =
	{
			{ L"WoodBox", CLevelDesign_Breakable::WOODBOX_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/BoxWood/BoxWood.ysh",
			MODEL::ANIM, 2u, "WoodBoxM__BoxWoodC", false, L"CommonHit", WOOD_BOX_BREAK_SOUNDKEY, 0.25f, BREAK_MASK_LV1 },
			{ L"BoxPlastic", CLevelDesign_Breakable::PLASTICBOX_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/BoxPlastic/BoxPlastic.ysh",
			MODEL::ANIM, 0u, "BoxPlasticM__BoxPlasticC", false, L"CommonHit", PLASTIC_BOX_BREAK_SOUNDKEY, 0.25f, BREAK_MASK_LV1 },
			{ L"BoxIron", CLevelDesign_Breakable::BOXIRON_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/BoxIron/BoxIron2.ysh",
			MODEL::ANIM, 0u, "Model__BoxIronC", false, L"CommonHit", nullptr, 0.f, BREAK_MASK_LV2 },
			{ L"BreakableRockS", CLevelDesign_Breakable::BREAKABLE_ROCK_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/BreakableRock/BreakableRock_S.ysh",
			MODEL::NONANIM, LD_INVALID_ID, nullptr, true, L"Split_Stone", NORMAL_ROCK_BREAK_SOUNDKEY, 0.45f, BREAK_MASK_LV2 },
			{ L"BreakableRockM", CLevelDesign_Breakable::BREAKABLE_ROCK_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/BreakableRock/BreakableRock_M.ysh",
			MODEL::NONANIM, LD_INVALID_ID, nullptr, true, L"Split_Stone_Big", NORMAL_ROCK_BREAK_SOUNDKEY, 0.45f, BREAK_MASK_LV2 },
			{ L"BreakableRockMForBridge", CLevelDesign_Breakable::BREAKABLE_ROCK_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/BreakableRock/BreakableRock_M.ysh",
			MODEL::NONANIM, LD_INVALID_ID, nullptr, true, L"Split_Stone_Big", LARGE_ROCK_BREAK_SOUNDKEY, 0.15f, BREAK_MASK_LV2 }
	};

	static const LD_BREAKABLE_CATALOG* Find_BreakableCatalog(const _wstring& wstrObjName)
	{
		for (const LD_BREAKABLE_CATALOG& Entry : g_BreakableCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

	void Build_DefaultBreakableDesc(LD_BREAKABLE_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return;

		*pOutDesc = {};

		pOutDesc->wstrSourcePath = L"Palette";
		pOutDesc->strSourceFile = L"BoxWood.ysh";
		pOutDesc->strSection = L"Palette";
		pOutDesc->strEntryKey = L"WoodBox_Default";
		pOutDesc->strObjectName = L"WoodBox";
		pOutDesc->strKind = L"Palette";

		pOutDesc->eCategory = LD_CATEGORY::BREAKABLE;
		pOutDesc->eModelType = MODEL::ANIM;
		pOutDesc->wstrModelProtoTag = CLevelDesign_Breakable::WOODBOX_MODEL_PROTO_TAG;

		pOutDesc->fScale = 1.f;
		pOutDesc->vRight = { 1.f, 0.f, 0.f, 0.f };
		pOutDesc->vUp = { 0.f, 1.f, 0.f, 0.f };
		pOutDesc->vLook = { 0.f, 0.f, 1.f, 0.f };
		pOutDesc->vPosition = { 0.f, 0.f, 0.f, 1.f };

		pOutDesc->vParsedPosition = { 0.f, 0.f, 0.f };
		pOutDesc->qParsedRotation = { 0.f, 0.f, 0.f, 1.f };
		pOutDesc->vParsedScale = { 1.f, 1.f, 1.f };
	}
}

NS_BEGIN(Client)

CLevelDesign_Breakable::CLevelDesign_Breakable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Breakable::CLevelDesign_Breakable(const CLevelDesign_Breakable& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tBreakableDesc(Prototype.m_tBreakableDesc)
{
}

HRESULT CLevelDesign_Breakable::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Breakable::Initialize(void* pArg)
{
	LD_BREAKABLE_DESC DefaultDesc{};
	if (nullptr == pArg)
	{
		Build_DefaultBreakableDesc(&DefaultDesc);
		pArg = &DefaultDesc;
	}

	m_tBreakableDesc = *static_cast<const LD_BREAKABLE_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_CullingState(m_pModelCom, BREAKABLE_CULL_MARGIN)))
		return E_FAIL;

	m_bUseShadow = true;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || nullptr == m_pHurtBox || nullptr == m_pRigidStatic)
		return E_FAIL;

	if (m_tBreakableDesc.eCategory != LD_CATEGORY::BREAKABLE)
		return E_FAIL;
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return E_FAIL;
	if (MODEL::ANIM != m_tBreakableDesc.eModelType && MODEL::NONANIM != m_tBreakableDesc.eModelType)
		return E_FAIL;
	if (BREAKABLE_STATE::INTACT != m_eState && BREAKABLE_STATE::BREAKING != m_eState &&
		BREAKABLE_STATE::DESTROYED != m_eState)
		return E_FAIL;

	const LD_BREAKABLE_CATALOG* pCatalog = Find_BreakableCatalog(m_tBreakableDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;
	if (pCatalog->eModelType != m_tBreakableDesc.eModelType)
		return E_FAIL;
	if (nullptr == pCatalog->pModelProtoTag || m_tBreakableDesc.wstrModelProtoTag !=
		pCatalog->pModelProtoTag)
		return E_FAIL;

	if (MODEL::ANIM == m_tBreakableDesc.eModelType)
	{
		if (nullptr == m_pAnimatorCom)
			return E_FAIL;
		if (nullptr == pCatalog->pBaseMeshName)
			return E_FAIL;

		const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
		if (LD_INVALID_ID == m_iBaseMeshIndex || m_iBaseMeshIndex >= iNumMeshes)
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Breakable::Update(_float fTimeDelta)
{
	if (MODEL::ANIM == m_tBreakableDesc.eModelType && BREAKABLE_STATE::BREAKING == m_eState)
	{
		m_pAnimatorCom->Update(fTimeDelta);

		if (m_pAnimatorCom->Is_Finished())
			m_eState = BREAKABLE_STATE::DESTROYED;
	}
}

void CLevelDesign_Breakable::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (BREAKABLE_STATE::DESTROYED == m_eState)
		return;

	if (BREAKABLE_STATE::INTACT == m_eState)
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}

	Check_Visible();
	Submit_RenderGroups();
}

HRESULT CLevelDesign_Breakable::Render()
{
	if (BREAKABLE_STATE::DESTROYED == m_eState)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		if (MODEL::ANIM == m_tBreakableDesc.eModelType)
		{
			MESH_LAYER_BIND_CONTEXT Ctx{};
			Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
			Ctx.iMesh = i;
			Ctx.pLayer = &Layer;
			Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_ANIM;
			Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
			Ctx.iFallbackPass = ETOUI(WORLD_PASS::DMN);

			MESH_LAYER_BIND_RESULT Result{};
			if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
				return E_FAIL;

			if (Result.bSkipMesh)
				continue;

			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(Result.iPass)))
				return E_FAIL;
		}
		else
		{
			MESH_LAYER_BIND_CONTEXT Ctx{};
			Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
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
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Render_Shadow()
{
	if (BREAKABLE_STATE::DESTROYED == m_eState)
		return S_OK;

	if (MODEL::NONANIM == m_tBreakableDesc.eModelType)
		return Render_ShadowModel(m_pShaderCom, m_pModelCom, MESH_LAYER_PROFILE::WORLD_NONANIM);

	if (MODEL::ANIM != m_tBreakableDesc.eModelType)
		return E_FAIL;

	if (FAILED(Bind_ShadowTransforms(m_pShaderCom)))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(Render_ShadowMesh(m_pShaderCom, m_pModelCom, i, MESH_LAYER_PROFILE::WORLD_ANIM)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Breakable::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLevelDesign_Breakable::Damaged(const ATTACK_INFO& tInfo)
{
	if (BREAKABLE_STATE::INTACT != m_eState)
		return;

	const LD_BREAKABLE_CATALOG* pCatalog = Find_BreakableCatalog(m_tBreakableDesc.strObjectName);
	if (nullptr == pCatalog)
		return;

	if (0 == (pCatalog->iBreakHitMask & Hit_Bit(tInfo.eHitType)))
		return;

	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();

	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam, XMVectorNegate(XMLoadFloat4(pCamLook)));

	_float3 vPos{};
	if (!Compute_EffectSpawnPosition(m_pModelCom, DESTROY_EFFECT_HEIGHT_RATIO, &vPos))
		XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (nullptr != pCatalog->pBreakSoundKey)
		m_pGameInstance_Proxy->Play_SFX(pCatalog->pBreakSoundKey, pCatalog->fVolume);

	if (nullptr != pCatalog->pDestroyEffectId)
	{
		if (MODEL::ANIM == pCatalog->eModelType)
			CEffect_Loader::GetInstance()->Spawn(pCatalog->pDestroyEffectId, Get_LevelIndex(), vPos, vFaceCam, _float3(0.f, 0.f, 0.f), nullptr);
		else
			CEffect_Loader::GetInstance()->Spawn(pCatalog->pDestroyEffectId, Get_LevelIndex(), vPos);
	}

	CEffect_Loader::GetInstance()->Spawn(L"CommonHit", Get_LevelIndex(), vPos, vFaceCam, _float3(0.f, 0.f, 0.f), nullptr);

	Release_RigidStatic();
	m_pHurtBox->Set_Enabled(false);

	Publish_LDEvent();

	if (MODEL::ANIM == m_tBreakableDesc.eModelType)
	{
		m_eState = BREAKABLE_STATE::BREAKING;
		m_pAnimatorCom->Set_PlaySpeed(1.5f);
		m_pModelCom->Set_AnimationIndex(m_iBreakAnimIndex, false, true, 0.f);
		return;
	}

	m_eState = BREAKABLE_STATE::DESTROYED;
}

void CLevelDesign_Breakable::Register_LevelDesignSpecs()
{
	for (const LD_BREAKABLE_CATALOG& Entry : g_BreakableCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
		Spec.eCategory = LD_CATEGORY::BREAKABLE;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
			{ Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType, Entry.bCookCollisionMesh }
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_Breakable::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;
	if (nullptr == Find_BreakableCatalog(CommonDesc.strObjectName))
		return false;
	if (Spec.eCategory != LD_CATEGORY::BREAKABLE || Spec.wstrModelProtoTag.empty())
		return false;

	LD_BREAKABLE_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_Breakable::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Breakable::Create(pDevice, pContext);
}

HRESULT CLevelDesign_Breakable::Ready_Components()
{
	const LD_BREAKABLE_CATALOG* pCatalog = Find_BreakableCatalog(m_tBreakableDesc.strObjectName);
	if (nullptr == pCatalog || pCatalog->eModelType != m_tBreakableDesc.eModelType)
		return E_FAIL;

	const _tchar* pModelProtoTag = Resolve_ModelProtoTag();
	if (nullptr == pModelProtoTag)
		return E_FAIL;

	const auto& ShaderDesc = MODEL::ANIM == m_tBreakableDesc.eModelType
		? Shader_World_Anim
		: Shader_World_NonAnim;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag,
		TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tBreakableDesc.iModelProtoLevel, pModelProtoTag,
		TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	if (MODEL::ANIM == m_tBreakableDesc.eModelType)
	{
		m_iBreakAnimIndex = pCatalog->iBreakAnimIndex;

		const _uint iNumAnimations = m_pModelCom->Get_NumAnimations();
		if (LD_INVALID_ID == m_iBreakAnimIndex || m_iBreakAnimIndex >= iNumAnimations)
			return E_FAIL;
		if (m_pModelCom->Get_AnimationName(m_iBreakAnimIndex) != "Break")
			return E_FAIL;
		if (nullptr == pCatalog->pBaseMeshName)
			return E_FAIL;

		const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
		for (_uint i = 0; i < iNumMeshes; ++i)
		{
			if (m_pModelCom->Get_MeshName(i) == pCatalog->pBaseMeshName)
			{
				m_iBaseMeshIndex = i;
				break;
			}
		}

		if (LD_INVALID_ID == m_iBaseMeshIndex)
			return E_FAIL;

		CAnimator::ANIMATOR_DESC AnimDesc{};
		AnimDesc.pModel = m_pModelCom;

		m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice,
			m_pContext));
		if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
			return E_FAIL;

		m_pModelCom->Update_Combined();
	}

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Ready_HurtBox()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vSize = { vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z };

	_float fRadius = vSize.x;
	if (fRadius < vSize.y)
		fRadius = vSize.y;
	if (fRadius < vSize.z)
		fRadius = vSize.z;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	ColliderDesc.fRadius = fRadius * 0.5f;

	m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"),
		&ColliderDesc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	m_pHurtBox->Set_OnEnter([this](CCollider* pOther)
		{
			if (nullptr == pOther)
				return;

			PLAYER_QUERY PlayerQuery{};
			m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &PlayerQuery);

			ATTACK_INFO AttackInfo{};
			if (!Try_ResolveContactHit(pOther->Get_RegisteredGroup(), pOther->Get_Owner(),
				PlayerQuery.pPlayer, &AttackInfo))
				return;

			Damaged(AttackInfo);
		});

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_HURT));

	return S_OK;
}

HRESULT CLevelDesign_Breakable::Ready_RigidStatic()
{
	Release_RigidStatic();

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vLocalCenter = {
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	const _float3 vLocalHalfExtents = {
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f
	};

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(
		vLocalCenter,
		vLocalHalfExtents,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Breakable::Release_RigidStatic()
{
	if (nullptr == m_pRigidStatic)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;
}

HRESULT CLevelDesign_Breakable::Bind_ShaderResources()
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

_bool CLevelDesign_Breakable::Should_RenderMesh(_uint iMeshIndex) const
{
	if (BREAKABLE_STATE::DESTROYED == m_eState)
		return false;

	if (MODEL::ANIM != m_tBreakableDesc.eModelType)
		return true;

	if (BREAKABLE_STATE::INTACT == m_eState)
		return iMeshIndex == m_iBaseMeshIndex;

	if (BREAKABLE_STATE::BREAKING == m_eState)
		return iMeshIndex != m_iBaseMeshIndex;

	return false;
}

const _tchar* CLevelDesign_Breakable::Resolve_ModelProtoTag() const
{
	if (m_tBreakableDesc.wstrModelProtoTag.empty())
		return nullptr;

	return m_tBreakableDesc.wstrModelProtoTag.c_str();
}

CLevelDesign_Breakable* CLevelDesign_Breakable::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Breakable* pInstance = new CLevelDesign_Breakable(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Breakable");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Breakable::Clone(void* pArg)
{
	CLevelDesign_Breakable* pInstance = new CLevelDesign_Breakable(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Breakable");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Breakable::Free()
{
	Release_RigidStatic();

	__super::Free();
}

NS_END
