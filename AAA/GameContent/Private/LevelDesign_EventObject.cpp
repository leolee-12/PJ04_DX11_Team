#include "LevelDesign_EventObject.h"
#include "LevelDesign_Registry.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"
#include "GameContrnt_Events.h"

#include "GameInstance.h"

namespace
{
	void Log_EventObjectPhysicsWarning(const string& strMessage)
	{
#ifdef _DEBUG
		OutputDebugStringA((strMessage + "\n").c_str());
#else
		UNREFERENCED_PARAMETER(strMessage);
#endif
	}

	inline constexpr _uint EVENTOBJECT_ANIM_DEFAULT_PASS = 1u;

	struct LD_EVENTOBJECT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		_string strAnimNames[LD_ANIM_SLOT_COUNT] = {};
		LD_EVENTOBJECT_POLICY ePolicy = { LD_EVENTOBJECT_POLICY::DEFAULT };
		_bool bUseCollMesh = true;
		const _tchar* pAnimEventFile = L"";
	};

	static const LD_EVENTOBJECT_CATALOG g_EventObjectCatalog[] =
	{
		{ L"Level1BossDemoBg", CLevelDesign_EventObject::LEVEL1BOSSDEMOBG_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg.ysh",
		MODEL::ANIM, { "DemoAppear2", "DemoAppear2AfterWait", "DemoAppear2BeforWait", "" }, LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG, true, L"../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg_AnimEvents.json" },
		{ L"SlopeBoardA", CLevelDesign_EventObject::SLOPEBOARD_A_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardA.ysh",
		MODEL::ANIM, { "LandBack", "LandFront", "LandStartFront", "" }, LD_EVENTOBJECT_POLICY::SLOPEBOARD_A, false, L"" },
		{ L"SlopeBoardC", CLevelDesign_EventObject::SLOPEBOARD_C_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardC.ysh",
		MODEL::ANIM, { "FallenWait", "Wait", "", "" }, LD_EVENTOBJECT_POLICY::SLOPEBOARD_C, true, L"" },
	};

	static const LD_EVENTOBJECT_CATALOG* Find_EventObjectCatalog(const _wstring& wstrObjName)
	{
		for (const LD_EVENTOBJECT_CATALOG& Entry : g_EventObjectCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

#pragma region LEVEL1_BOSS_DEMO_BG
	inline constexpr _uint LEVEL1_BOSS_DEMO_BG_GLASS_PASS = 0u;

	_bool Is_Level1BossDemoBgGlassMesh(const string& strMeshName)
	{
		return "GlassSideM__MlBossGlassC" == strMeshName || "GlassM__MlBossGlassC" == strMeshName;
	}
#pragma endregion

#pragma region SLOPEBOARD_A
	inline constexpr const _char* s_strSlopeBoardA_PlatformMeshName = "PlateM__BoardC";
	inline constexpr const _char* s_strSlopeBoardA_PlatformBoneName = "FrontAnimL";
	inline constexpr _float s_strSlopeBoardA_TriggerMargin = 0.f;
	inline constexpr _float s_strSlopeBoardA_MinHalfExtent = 0.01f;

	_matrix RemoveScale_ForSlopeBoardBox(_fmatrix WorldMatrix)
	{
		XMVECTOR vScale{}, vRotation{}, vTranslation{};
		if (!XMMatrixDecompose(&vScale, &vRotation, &vTranslation, WorldMatrix))
			return WorldMatrix;

		return XMMatrixRotationQuaternion(XMQuaternionNormalize(vRotation)) * XMMatrixTranslationFromVector(vTranslation);
	}
#pragma endregion
}

NS_BEGIN(Client)

CLevelDesign_EventObject::CLevelDesign_EventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_EventObject::CLevelDesign_EventObject(const CLevelDesign_EventObject& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tEventObjectDesc(Prototype.m_tEventObjectDesc)
{
}

HRESULT CLevelDesign_EventObject::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_EventObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tEventObjectDesc = *static_cast<const LD_EVENTOBJECT_DESC*>(pArg);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_AnimEvents()))
		return E_FAIL;

	if (FAILED(Ready_Policy()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_EventObject::Update(_float fTimeDelta)
{
	if (m_bAnimationActive && m_pAnimatorCom)
		m_pAnimatorCom->Update(fTimeDelta);

	Update_Policy(fTimeDelta);
}

void CLevelDesign_EventObject::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bRenderable)
		return;

	if (m_pInteractionTrigger && m_pInteractionTrigger->Is_Enabled())
	{
		_matrix TriggerWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

		if (LD_EVENTOBJECT_POLICY::SLOPEBOARD_A == m_tEventObjectDesc.ePolicy && nullptr != m_pModelCom)
		{
			const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr(s_strSlopeBoardA_PlatformBoneName);
			if (nullptr != pBoneMatrix)
				TriggerWorld = RemoveScale_ForSlopeBoardBox(XMLoadFloat4x4(pBoneMatrix) * TriggerWorld);
		}

		m_pInteractionTrigger->Update(TriggerWorld);
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractionTrigger);
#endif
	}

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_EventObject::Render()
{
	if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (!Should_RenderMesh_ByPolicy(i))	// 렌더하지 않을 메쉬는 스킵
			continue;

		const _uint iPass = Resolve_RenderPass_ByPolicy(i);	// 메쉬별 패스 설정

		if (FAILED(Render_Mesh(i, iPass)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_EventObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

_bool CLevelDesign_EventObject::Play_EventAnimation(_uint iAnimSlot, _bool bLoop)
{
	if (LD_ANIM_SLOT_COUNT <= iAnimSlot)
		return false;

	return Play_EventAnimation(m_tEventObjectDesc.strAnimNames[iAnimSlot], bLoop);
}

_bool CLevelDesign_EventObject::Play_EventAnimation(const _string& strAnimName, _bool bLoop)
{
	if (nullptr == m_pAnimatorCom || nullptr == m_pModelCom || strAnimName.empty())
		return false;
	if (m_pModelCom->Get_AnimationIndex(strAnimName) < 0)
		return false;

	m_pAnimatorCom->Resume();
	//m_pAnimatorCom->Play(strAnimName, bLoop, true);
	m_pAnimatorCom->Play(strAnimName, bLoop, true, 0.f, 1.5f);
	m_bAnimationActive = true;
	return true;
}

void CLevelDesign_EventObject::Register_LevelDesignSpecs()
{
	for (const LD_EVENTOBJECT_CATALOG& Entry : g_EventObjectCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
		Spec.eCategory = LD_CATEGORY::GIMMICK;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements =
		{
			  { Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType, Entry.bUseCollMesh },
		};

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_EventObject::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;
	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.wstrModelProtoTag.empty())
		return false;

	const LD_EVENTOBJECT_CATALOG* pCatalog = Find_EventObjectCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = pCatalog->strAnimNames[i];

	Desc.ePolicy = pCatalog->ePolicy;
	Desc.bUseCollMesh = pCatalog->bUseCollMesh;
	Desc.strAnimEventFile = pCatalog->pAnimEventFile ? pCatalog->pAnimEventFile : L"";

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_EventObject::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_EventObject::Create(pDevice, pContext);
}

HRESULT CLevelDesign_EventObject::Validate_Desc()
{
	if (m_tEventObjectDesc.eCategory != LD_CATEGORY::GIMMICK)
		return E_FAIL;
	if (m_tEventObjectDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_Components()
{
	if (m_tEventObjectDesc.wstrModelProtoTag.empty())
		return E_FAIL;

	const auto& ShaderDesc = MODEL::ANIM == m_tEventObjectDesc.eModelType ? Shader_AnimMesh_PBR : Shader_NonAnimMesh_PBR;

	m_pShaderCom = Add_Component<CShader>(ShaderDesc.iLevelID, ShaderDesc.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tEventObjectDesc.iModelProtoLevel, m_tEventObjectDesc.wstrModelProtoTag.c_str(),
		TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		CAnimator::ANIMATOR_DESC AnimDesc{};
		AnimDesc.pModel = m_pModelCom;
		AnimDesc.strDataFile = m_tEventObjectDesc.strAnimEventFile;
		m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
		if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_AnimEvents()
{
	if (nullptr == m_pAnimatorCom)
		return S_OK;

	m_pAnimatorCom->Set_EventCallback(
		[this](const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
		{
			On_AnimEvent(AnimEvent, ePhase);
		});

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_RigidStatic()
{
	Release_RigidStatic();

	if (!m_tEventObjectDesc.bUseCollMesh)
		return S_OK;

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] RigidStatic skipped: required component is null.");
		return E_FAIL;
	}

	auto pCollisionMesh = m_pModelCom->Get_CollisionMesh();
	if (nullptr == pCollisionMesh)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] RigidStatic skipped: cooked collision mesh is null.");
		return E_FAIL;
	}

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(pCollisionMesh, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	if (nullptr == m_pRigidStatic)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] RigidStatic failed: Create_StaticActor returned null.");
		return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_EventObject::Release_RigidStatic()
{
	if (nullptr == m_pRigidStatic)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;
}

void CLevelDesign_EventObject::Set_RigidStaticEnabled(_bool bEnable)
{
	if (!bEnable)
	{
		Release_RigidStatic();
		return;
	}

	if (nullptr != m_pRigidStatic)
		return;

	if (FAILED(Ready_RigidStatic()))
		Release_RigidStatic();
}

_int CLevelDesign_EventObject::Find_MeshIndex_ByName(const _string& strMeshName) const
{
	if (nullptr == m_pModelCom)
		return -1;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (m_pModelCom->Get_MeshName(i) == strMeshName)
			return static_cast<_int>(i);
	}

	return -1;
}

#ifdef _DEBUG
void CLevelDesign_EventObject::Debug_DumpSlopeBoardPlatformBinding()
{
	if (LD_EVENTOBJECT_POLICY::SLOPEBOARD_A != m_tEventObjectDesc.ePolicy || nullptr == m_pModelCom)
		return;

	const _int iMeshIndex = Find_MeshIndex_ByName(s_strSlopeBoardA_PlatformMeshName);
	const _int iBoneIndex = m_pModelCom->Get_BoneIndex(s_strSlopeBoardA_PlatformBoneName);

	string strHeader = "[SlopeBoardA] platform mesh=" + string(s_strSlopeBoardA_PlatformMeshName) +
		" meshIndex=" + to_string(iMeshIndex) + " bone=" + string(s_strSlopeBoardA_PlatformBoneName) +
		" boneIndex=" + to_string(iBoneIndex) + "\n";
	OutputDebugStringA(strHeader.c_str());

	for (_uint i = 0; i < static_cast<_uint>(m_pModelCom->Get_NumMeshes()); ++i)
	{
		string strLine = "[SlopeBoardA] mesh[" + to_string(i) + "] " + m_pModelCom->Get_MeshName(i) + "\n";
		OutputDebugStringA(strLine.c_str());
	}

	for (_uint i = 0; i < m_pModelCom->Get_NumBones(); ++i)
	{
		string strLine = "[SlopeBoardA] bone[" + to_string(i) + "] " + m_pModelCom->Get_BoneName(i) + "\n";
		OutputDebugStringA(strLine.c_str());
	}
}
#endif

HRESULT CLevelDesign_EventObject::Ready_RigidStatic_FromMeshAABB(const _string& strMeshName, const _string& strBoneName)
{
	Release_RigidStatic();

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
		return E_FAIL;

	const _int iMeshIndex = Find_MeshIndex_ByName(strMeshName);
	if (0 > iMeshIndex)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] mesh not found: " + strMeshName);
		return E_FAIL;
	}

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(static_cast<_uint>(iMeshIndex), &vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] invalid mesh AABB: " + strMeshName);
		return E_FAIL;
	}

	const _float3 vLocalCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	_float3 vLocalHalfExtents = { (vMax.x - vMin.x) * 0.5f, (vMax.y - vMin.y) * 0.5f, (vMax.z - vMin.z) * 0.5f };

	vLocalHalfExtents.x = max(vLocalHalfExtents.x, s_strSlopeBoardA_MinHalfExtent);
	vLocalHalfExtents.y = max(vLocalHalfExtents.y, s_strSlopeBoardA_MinHalfExtent);
	vLocalHalfExtents.z = max(vLocalHalfExtents.z, s_strSlopeBoardA_MinHalfExtent);

	_matrix MeshWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName);
	if (nullptr != pBoneMatrix)
		MeshWorld = XMLoadFloat4x4(pBoneMatrix) * MeshWorld;
	else
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] bone not found. Use object world: " + strBoneName);

	MeshWorld = RemoveScale_ForSlopeBoardBox(MeshWorld);

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(vLocalCenter, vLocalHalfExtents, MeshWorld);
	if (nullptr == m_pRigidStatic)
	{
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] Create_StaticBox failed: " + strMeshName);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_SlopeBoardTrigger()
{
	if (nullptr == m_pModelCom || nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	const _int iMeshIndex = Find_MeshIndex_ByName(s_strSlopeBoardA_PlatformMeshName);
	if (0 > iMeshIndex)
		return E_FAIL;

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(static_cast<_uint>(iMeshIndex), &vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	const _float3 vSize = {
		  (vMax.x - vMin.x) + s_strSlopeBoardA_TriggerMargin * 2.f,
		  (vMax.y - vMin.y) + s_strSlopeBoardA_TriggerMargin * 2.f,
		  (vMax.z - vMin.z) + s_strSlopeBoardA_TriggerMargin * 2.f
	};

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.vSize = vSize;

	m_pInteractionTrigger = Add_Component<CCollider>(Collider_OBB.iLevelID, Collider_OBB.szProtoTag, TEXT("Com_SlopeBoardTrigger"),
		&ColliderDesc);
	if (nullptr == m_pInteractionTrigger)
		return E_FAIL;

	SetUp_SlopeBoardTriggerCallback();
	m_pGameInstance_Proxy->Register_Collider(m_pInteractionTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));
	m_bInteractionTriggerRegistered = true;

	return S_OK;
}

void CLevelDesign_EventObject::SetUp_SlopeBoardTriggerCallback()
{
	if (nullptr == m_pInteractionTrigger)
		return;

	m_pInteractionTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_SlopeBoardTrigger(pOther); });
}

void CLevelDesign_EventObject::Handle_SlopeBoardTrigger(CCollider* pOther)
{
	if (nullptr == pOther)
		return;
	if (LD_EVENTOBJECT_POLICY::SLOPEBOARD_A != m_tEventObjectDesc.ePolicy)
		return;
	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return;

	On_Event_SlopeBoard(L"");
}

void CLevelDesign_EventObject::Unregister_SlopeBoardTrigger(_bool bImmediate)
{
	if (nullptr == m_pInteractionTrigger)
		return;

	m_pInteractionTrigger->Set_Enabled(false);

	if (!m_bInteractionTriggerRegistered)
		return;

	const _uint iGroup = ETOUI(COLLISION_LAYER::ENV_TRIGGER);
	if (bImmediate)
		m_pGameInstance_Proxy->Immediate_Unregister(m_pInteractionTrigger, iGroup);
	else
		m_pGameInstance_Proxy->Request_Unregister(m_pInteractionTrigger, iGroup);

	m_bInteractionTriggerRegistered = false;
}

HRESULT CLevelDesign_EventObject::Ready_Policy()
{
	if (m_pModelCom)
		m_MeshVisible.assign(static_cast<_uint>(m_pModelCom->Get_NumMeshes()), true);
	else
		m_MeshVisible.clear();

	switch (m_tEventObjectDesc.ePolicy)
	{
	case LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG:
		return Ready_Level1BossDemoBg();

	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_A:
		return Ready_SlopeBoardA();

	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_C:
		return Ready_SlopeBoardC();

	case LD_EVENTOBJECT_POLICY::DEFAULT:
	default:
		break;
	}

	m_eState = EVENTOBJECT_STATE::IDLE;
	m_bRenderable = true;
	return S_OK;
}

void CLevelDesign_EventObject::Update_Policy(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bAnimationActive || nullptr == m_pAnimatorCom)
		return;

	if (!m_pAnimatorCom->Is_Finished())
		return;

	m_bAnimationActive = false;

	switch (m_tEventObjectDesc.ePolicy)
	{
	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_A:
		if (EVENTOBJECT_STATE::PLAYING == m_eState)
		{
			m_eState = EVENTOBJECT_STATE::BROKEN;
			if (FAILED(Ready_RigidStatic_FromMeshAABB(s_strSlopeBoardA_PlatformMeshName, s_strSlopeBoardA_PlatformBoneName)))
				Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] SlopeBoardA platform box failed.");
		}
		break;

	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_C:
		if (EVENTOBJECT_STATE::PLAYING == m_eState)
			m_eState = EVENTOBJECT_STATE::BROKEN;
		break;

	default:
		break;
	}
}

HRESULT CLevelDesign_EventObject::Ready_Events()
{
	switch (m_tEventObjectDesc.ePolicy)
	{
	case LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG:
		Subscribe_Event(EventTag::Cutscene_GorillaBreak, [this](void*) { On_Event(EventTag::Cutscene_GorillaBreak); });
		break;

	default:
		break;
	}

	return S_OK;
}

void CLevelDesign_EventObject::On_Event(const _wstring& strEventTag)
{
	switch (m_tEventObjectDesc.ePolicy)
	{
	case LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG:
		On_Event_Level1BossDemoBg(strEventTag);
		break;

	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_A:
	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_C:
		On_Event_SlopeBoard(strEventTag);
		break;

	default:
		break;
	}
}

void CLevelDesign_EventObject::On_Event_Level1BossDemoBg(const _wstring& strEventTag)
{
	UNREFERENCED_PARAMETER(strEventTag);

	m_eState = EVENTOBJECT_STATE::PLAYING;
	Play_EventAnimation(0u, false);
}

void CLevelDesign_EventObject::On_Event_SlopeBoard(const _wstring& strEventTag)
{
	UNREFERENCED_PARAMETER(strEventTag);

	if (EVENTOBJECT_STATE::IDLE != m_eState)
		return;

	if (!Play_EventAnimation(0u, false))
		return;

	m_eState = EVENTOBJECT_STATE::PLAYING;
	m_bRenderable = true;
	Set_AllMeshesVisible(true);
	Unregister_SlopeBoardTrigger(false);
	Release_RigidStatic();
}

void CLevelDesign_EventObject::On_AnimEvent(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
{
	switch (m_tEventObjectDesc.ePolicy)
	{
	case LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG:
		On_AnimEvent_Level1BossDemoBg(AnimEvent, ePhase);
		break;

	default:
		break;
	}
}

void CLevelDesign_EventObject::On_AnimEvent_Level1BossDemoBg(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
{
	if (ANIM_EVENT_PHASE::POINT != ePhase)
		return;

	switch (static_cast<EANIM_EVENT>(AnimEvent.iEventType))
	{
	case EANIM_EVENT::PubEvent:
	{
		if (AnimEvent.strParam.empty())
			break;

		const _wstring strEventTag(AnimEvent.strParam.begin(), AnimEvent.strParam.end());
		m_pGameInstance_Proxy->Publish(strEventTag, nullptr);
		break;
	}

	case EANIM_EVENT::OnOffMesh:
	{
		Set_AllMeshesVisible(true);

		static const _uint AfterMeshIdx[] = { 47, 48 };
		for (_uint idx : AfterMeshIdx)
			Set_MeshVisible(idx, false);

		break;
	}

	default:
		break;
	}
}

void CLevelDesign_EventObject::Set_AllMeshesVisible(_bool bVisible)
{
	for (_uint i = 0; i < static_cast<_uint>(m_MeshVisible.size()); ++i)
		m_MeshVisible[i] = bVisible;
}

void CLevelDesign_EventObject::Set_MeshVisible(_uint iMeshIndex, _bool bVisible)
{
	if (iMeshIndex >= m_MeshVisible.size())
		return;

	m_MeshVisible[iMeshIndex] = bVisible;
}

_bool CLevelDesign_EventObject::Should_RenderMesh_ByPolicy(_uint iMeshIndex) const
{
	if (iMeshIndex < m_MeshVisible.size() && !m_MeshVisible[iMeshIndex])
		return false;

	switch (m_tEventObjectDesc.ePolicy)
	{
	case LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG:
	{
		if(Is_Level1BossDemoBgGlassMesh(m_pModelCom->Get_MeshName(iMeshIndex))) // temp
			return false;
	}
		return true;

	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_A:
	case LD_EVENTOBJECT_POLICY::SLOPEBOARD_C:
		if (EVENTOBJECT_STATE::BROKEN == m_eState)
			return 0u != iMeshIndex;
		return true;

	default:
		return true;
	}
}

_uint CLevelDesign_EventObject::Resolve_RenderPass_ByPolicy(_uint iMeshIndex) const
{
	if (LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG == m_tEventObjectDesc.ePolicy
		&& Is_Level1BossDemoBgGlassMesh(m_pModelCom->Get_MeshName(iMeshIndex)))
		return LEVEL1_BOSS_DEMO_BG_GLASS_PASS;

	return EVENTOBJECT_ANIM_DEFAULT_PASS;
}

HRESULT CLevelDesign_EventObject::Ready_Level1BossDemoBg()
{
	m_eState = EVENTOBJECT_STATE::IDLE;
	m_bRenderable = true;

	static _uint PieceIdx[] = { 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46 };

	for (auto idx : PieceIdx)
		Set_MeshVisible(idx, false);

	if (nullptr != m_pAnimatorCom)
		Play_EventAnimation(2u, false);

	return Ready_RigidStatic();
}

HRESULT CLevelDesign_EventObject::Ready_SlopeBoardA()
{
	m_eState = EVENTOBJECT_STATE::IDLE;
	m_bRenderable = true;

	if (nullptr == m_pAnimatorCom)
		return E_FAIL;

#ifdef _DEBUG
	Debug_DumpSlopeBoardPlatformBinding();
#endif

	if (FAILED(Ready_RigidStatic_FromMeshAABB(s_strSlopeBoardA_PlatformMeshName, s_strSlopeBoardA_PlatformBoneName)))
		Log_EventObjectPhysicsWarning("[LDEventObjectPhysics] SlopeBoardA initial platform box failed.");

	if (FAILED(Ready_SlopeBoardTrigger()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Ready_SlopeBoardC()
{
	m_eState = EVENTOBJECT_STATE::IDLE;
	m_bRenderable = true;

	if (nullptr == m_pAnimatorCom)
		return E_FAIL;

	//if (!Play_EventAnimation(1u, false))
	//	return E_FAIL;

	return Ready_RigidStatic();
}

HRESULT CLevelDesign_EventObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_EventObject::Render_Mesh(_uint iMeshIndex)
{
	return Render_Mesh(iMeshIndex, EVENTOBJECT_ANIM_DEFAULT_PASS);
}

HRESULT CLevelDesign_EventObject::Render_Mesh(_uint iMeshIndex, _uint iAnimPassIndex)
{
	const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(iMeshIndex);

	auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
		{
			const _uint iLayerIndex = MTEX_TYPE::UNKNOWN == eType ? 3u : Layer.idx[ETOUI(eType)];
			const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(iMeshIndex, eType);

			if (0u < iTextureCount)
			{
				const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

				if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pConstantName, iMeshIndex, eType, iSafeIndex)))
					return S_OK;
			}

			return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
		};

	if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))             return E_FAIL;
	if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))			return E_FAIL;
	if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))                   return E_FAIL;
	if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))               return E_FAIL;

	if (MODEL::ANIM == m_tEventObjectDesc.eModelType)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIndex)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(iAnimPassIndex)))
			return E_FAIL;
	}
	else
	{
		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float)))) return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN))) return E_FAIL;
	}

	if (FAILED(m_pModelCom->Render(iMeshIndex)))
		return E_FAIL;

	return S_OK;
}

CLevelDesign_EventObject* CLevelDesign_EventObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_EventObject* pInstance = new CLevelDesign_EventObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_EventObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_EventObject::Clone(void* pArg)
{
	CLevelDesign_EventObject* pInstance = new CLevelDesign_EventObject(*this);

	//LD_EVENTOBJECT_DESC TempDesc{};	// Test
	//if (nullptr == pArg)
	//{
	//	TempDesc.strObjectName = L"Level1BossDemoBg";
	//	TempDesc.strKind = L"Level1BossDemoBg";
	//	TempDesc.eCategory = LD_CATEGORY::GIMMICK;
	//	TempDesc.iModelProtoLevel = ETOUI(LEVEL::GAMEPLAY);
	//	TempDesc.eModelType = MODEL::ANIM;
	//	TempDesc.wstrModelProtoTag = LEVEL1BOSSDEMOBG_MODEL_PROTO_TAG;
	//	TempDesc.ePolicy = LD_EVENTOBJECT_POLICY::LEVEL1_BOSS_DEMO_BG;
	//	TempDesc.bUseCollMesh = false;
	//	pArg = &TempDesc;
	//}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_EventObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_EventObject::Free()
{
	Unregister_SlopeBoardTrigger(true);
	Release_RigidStatic();

	__super::Free();
}

NS_END