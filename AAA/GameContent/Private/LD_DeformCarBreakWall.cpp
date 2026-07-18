#include "LD_DeformCarBreakWall.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"
#include "Effect_Loader.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"

namespace
{
	inline constexpr const _char*	DEFORM_CAR_BREAK_WALL_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/DeformCarBreakWall/DeformCarBreakWall.ysh";
	inline constexpr const _char*	ANIM_FIRST = "DeformCarGetFirst";
	inline constexpr const _char*	ANIM_END = "DeformCarGetEnd";
	inline constexpr const _char*	DEFORM_CAR_BREAK_WALL_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { ANIM_FIRST, ANIM_END, "", "" };
	inline constexpr const _tchar*	BREAK_WALL_EFFECT_ID = L"BreakWallEffect";
	inline constexpr const _float3  BREAK_WALL_EFFECT_OFFSET[] = {
		{-3.7f, 8.4f, -2.75f},
		{+3.5f, 9.0f, -2.75f},
		{-3.4f, 9.6f, -2.75f},
	};
	
	inline constexpr _float DEFORM_CAR_BREAK_WALL_ANIM_SPEED = 1.5f;
	inline constexpr const _uint DEFORM_CAR_BREAK_WALL_MESH_COUNT = 31u;
	inline constexpr const _uint DEFORM_CAR_BREAK_WALL_COLLIMESH_INDEX = 10u;
	inline constexpr const _uint DEFORM_CAR_BREAK_WALL_CRUCK_SHADE_MESH_INDEX = 2u;

	inline constexpr const _uint DISABLE_MESH_INDICES[] = { 1 };
	inline constexpr const _uint ON_TO_OFF_MESH_INDICES[] = { 0,2 };
	inline constexpr const _uint OFF_TO_ON_MESH_INDICES[] = { 3,4,5,6,7,8,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 };
	
	inline constexpr const _uint ON_TO_OFF_AT_FRAME_MESH_INDICES[] = { 9,10 };
	inline constexpr const _float MESH_HIDE_TRACK_FRAME = 2.f;
}

NS_BEGIN(Client)

CLD_DeformCarBreakWall::CLD_DeformCarBreakWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_DeformCarBreakWall::CLD_DeformCarBreakWall(const CLD_DeformCarBreakWall& Prototype)
	: CLD_EventObject(Prototype)
	, m_eState(Prototype.m_eState)
{
}

HRESULT CLD_DeformCarBreakWall::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	const LD_ANIM_PLAY_DESC AnimDescs[] =
	{
		{ ANIM_FIRST, false, DEFORM_CAR_BREAK_WALL_ANIM_SPEED },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	if (FAILED(Set_AnimPose(ANIM_FIRST, 0.f)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_DeformCarBreakWall::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tEventObjectDesc.bUseCollMesh || !m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
	{
		if (m_tEventObjectDesc.strAnimNames[i] != DEFORM_CAR_BREAK_WALL_ANIM_NAMES[i])
			return E_FAIL;
	}

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	if (STATE::IDLE != m_eState)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	return S_OK;
}

void CLD_DeformCarBreakWall::Update(_float fTimeDelta)
{
	const _bool bAnimationWasActive = m_bAnimationActive;

	__super::Update(fTimeDelta);

	if (STATE::BREAKING == m_eState && !m_bMeshHiddenAtFrame)
	{
		const _int iAnimationIndex = m_pModelCom->Get_AnimationIndex(ANIM_FIRST);
		if (0 <= iAnimationIndex)
		{
			const _float fDuration = m_pModelCom->Get_AnimationDuration(static_cast<_uint>(iAnimationIndex));
			const _float fFrameOneProgress = 0.f < fDuration ? MESH_HIDE_TRACK_FRAME / fDuration : 1.f;

			if (m_pModelCom->Get_CurrentAnimProgress() >= fFrameOneProgress)
			{
				for (_uint iMeshIndex : ON_TO_OFF_AT_FRAME_MESH_INDICES)
					Set_MeshVisible(iMeshIndex, false);

				m_bMeshHiddenAtFrame = true;
			}
		}
	}

	if (STATE::BREAKING == m_eState && m_pAnimatorCom->Is_Finished())
	{
		_bool bShow = true;
		m_pGameInstance_Proxy->Publish(EventTag::HUD_SetVisible, &bShow);
	}

	if (bAnimationWasActive
		&& !m_bAnimationActive
		&& STATE::BREAKING == m_eState)
	{
		KIRBY_ATTACHMENT_END_DESC tDesc{};
		tDesc.eType = KIRBY_ATTACHMENT_END_REASON::DEFORM_CAR_GET_FIRST_RELEASE;
		m_pGameInstance_Proxy->Publish(EventTag::Kirby_AttachmentEnd, &tDesc);

		CUTSCENE_CAMERA_DESC cam{};
		cam.eCam = ECutsceneCam::Area;
		m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
		m_pGameInstance_Proxy->Publish(EventTag::Letterbox_End, nullptr);
		m_eState = STATE::BROKEN;
	}
}

void CLD_DeformCarBreakWall::Late_Update(_float fTimeDelta)
{
	if (STATE::IDLE == m_eState && m_pTrigger->Is_Enabled())
	{
		m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

HRESULT CLD_DeformCarBreakWall::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (DEFORM_CAR_BREAK_WALL_CRUCK_SHADE_MESH_INDEX == i || !Should_RenderMesh(i))
			continue;

		if (FAILED(Render_Mesh(i, Resolve_RenderPass(i))))
			return E_FAIL;
	}

	if (Should_RenderMesh(DEFORM_CAR_BREAK_WALL_CRUCK_SHADE_MESH_INDEX))
	{
		const _uint iPass = Resolve_RenderPass(DEFORM_CAR_BREAK_WALL_CRUCK_SHADE_MESH_INDEX);
		if (FAILED(Render_Mesh(DEFORM_CAR_BREAK_WALL_CRUCK_SHADE_MESH_INDEX, iPass)))
			return E_FAIL;
	}

	return S_OK;
}

void CLD_DeformCarBreakWall::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_DeformCarBreakWall::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::ANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements = { { MODEL_PROTO_TAG, DEFORM_CAR_BREAK_WALL_MODEL_PATH, MODEL::ANIM, false }, };

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_DeformCarBreakWall::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.eModelType != MODEL::ANIM || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = false;
	Desc.strAnimEventFile.clear();

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = DEFORM_CAR_BREAK_WALL_ANIM_NAMES[i];

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_DeformCarBreakWall::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_DeformCarBreakWall::Create(pDevice, pContext);
}

HRESULT CLD_DeformCarBreakWall::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_RenderComponent()))
		return E_FAIL;

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(DEFORM_CAR_BREAK_WALL_COLLIMESH_INDEX, &vMin, &vMax);

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
		return E_FAIL;

	const BoundingBox LocalBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);
	if (LocalBounds.Extents.x <= 0.f || LocalBounds.Extents.y <= 0.f || LocalBounds.Extents.z <= 0.f)
		return E_FAIL;

	if (FAILED(Ready_WallRigidStatic(LocalBounds)))
		return E_FAIL;

	if (FAILED(Ready_Trigger(LocalBounds)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_DeformCarBreakWall::Ready_RenderComponent()
{
	m_eState = STATE::IDLE;

	if (DEFORM_CAR_BREAK_WALL_MESH_COUNT != static_cast<_uint>(m_pModelCom->Get_NumMeshes()))
		return E_FAIL;

	for (_uint iMeshIndex : DISABLE_MESH_INDICES)
		Set_MeshVisible(iMeshIndex, false);

	for (_uint iMeshIndex : ON_TO_OFF_MESH_INDICES)
		Set_MeshVisible(iMeshIndex, true);

	for (_uint iMeshIndex : OFF_TO_ON_MESH_INDICES)
		Set_MeshVisible(iMeshIndex, false);

	for (_uint iMeshIndex : ON_TO_OFF_AT_FRAME_MESH_INDICES)
		Set_MeshVisible(iMeshIndex, true);

	m_bMeshHiddenAtFrame = false;

	return S_OK;
}

HRESULT CLD_DeformCarBreakWall::Ready_WallRigidStatic(const BoundingBox& LocalBounds)
{
	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(
		LocalBounds.Center, LocalBounds.Extents, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	return nullptr != m_pRigidStatic ? S_OK : E_FAIL;
}

HRESULT CLD_DeformCarBreakWall::Ready_Trigger(const BoundingBox& LocalBounds)
{
	BoundingBox TriggerBounds = LocalBounds;

	constexpr _float fPadding = 2.f;
	if (!GeometryUtils::Expand_AABB(&TriggerBounds, fPadding))
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = TriggerBounds.Center;
	ColliderDesc.vSize = { TriggerBounds.Extents.x * 2.f, TriggerBounds.Extents.y * 2.f, TriggerBounds.Extents.z * 2.f };

	if (ColliderDesc.vSize.x <= 0.f || ColliderDesc.vSize.y <= 0.f || ColliderDesc.vSize.z <= 0.f)
		return E_FAIL;

	m_pTrigger = Add_Component<CCollider>(Collider_OBB.iLevelID, Collider_OBB.szProtoTag, TEXT("Com_Trigger"),
		&ColliderDesc);

	if (nullptr == m_pTrigger)
		return E_FAIL;

	SetUp_Collider_Callback();

	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_DeformCarBreakWall::On_Event()
{
	if (STATE::IDLE != m_eState)
		return;

	Play_Anim(ANIM_FIRST);

	_float3 vEffectPos{};
	_float3 vEffectMin{}, vEffectMax{};
	m_pModelCom->Get_MeshAABB(DEFORM_CAR_BREAK_WALL_COLLIMESH_INDEX, &vEffectMin, &vEffectMax);

	if (GeometryUtils::Is_ValidAABB(vEffectMin, vEffectMax))
	{
		const BoundingBox EffectBounds = GeometryUtils::Make_AABB_FromMinMax(vEffectMin, vEffectMax);
		XMStoreFloat3(&vEffectPos, XMVector3TransformCoord(
			XMLoadFloat3(&EffectBounds.Center), XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));
	}
	else
		XMStoreFloat3(&vEffectPos, m_pTransformCom->Get_State(Engine::STATE::POSITION));
	
	for (_uint i = 0; i < 3; ++i)
	{
		_float3 vPos = vEffectPos;
		vPos.x += BREAK_WALL_EFFECT_OFFSET[i].x;
		vPos.y += BREAK_WALL_EFFECT_OFFSET[i].y;
		vPos.z += BREAK_WALL_EFFECT_OFFSET[i].z;
		CEffect_Loader::GetInstance()->Spawn(BREAK_WALL_EFFECT_ID, Get_LevelIndex(), vPos);
	}

	for (_uint iMeshIndex : ON_TO_OFF_MESH_INDICES)
		Set_MeshVisible(iMeshIndex, false);

	for (_uint iMeshIndex : OFF_TO_ON_MESH_INDICES)
		Set_MeshVisible(iMeshIndex, true);

	m_eState = STATE::BREAKING;

	KIRBY_ATTACHMENT_BEGIN_DESC Desc{};
	Desc.eType = KIRBY_ATTACHMENT_CONTEXT::DEFORM_CAR_GET_FIRST;
	Desc.pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr("CarConstL");
	Desc.pSourceWorld = m_pTransformCom->Get_WorldMatrixPtr();
	m_pGameInstance_Proxy->Publish(EventTag::Kirby_AttachmentBegin, &Desc);

	CUTSCENE_CAMERA_DESC cam{};
	cam.eCam = ECutsceneCam::Cutscene;
	cam.szTrack = L"DeformCarGetFirst_camera1";
	cam.pProgress = m_pAnimatorCom;
	cam.pAnchorWorld = m_pTransformCom->Get_WorldMatrixPtr();
	m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);

	CAMERA_SHAKE_DESC Shake{};
	Shake.fTrauma = 1.f;
	Shake.bIgnoreTimeScale = true;
	m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &Shake);

	_bool bShow = false;
	m_pGameInstance_Proxy->Publish(EventTag::HUD_SetVisible, &bShow);
	m_pGameInstance_Proxy->Publish(EventTag::Letterbox_Begin, nullptr);

	m_pGameInstance_Proxy->Play_SFX(L"DemoDeformCarGetFirst_BreakGroundWall.wav", 0.5f, ESoundBus::SFX);

	m_pGameInstance_Proxy->Lerp_TimeScale(0.1f, 1.f, 3.f);

	Release_RigidStatic();

	m_pTrigger->Set_Enabled(false);
}

void CLD_DeformCarBreakWall::SetUp_Collider_Callback()
{
	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_TriggerEnter(pOther); });
}

void CLD_DeformCarBreakWall::Handle_TriggerEnter(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE) != pOther->Get_RegisteredGroup())
		return;

	On_Event();
}

CLD_DeformCarBreakWall* CLD_DeformCarBreakWall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_DeformCarBreakWall* pInstance = new CLD_DeformCarBreakWall(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_DeformCarBreakWall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_DeformCarBreakWall::Clone(void* pArg)
{
	CLD_DeformCarBreakWall* pInstance = new CLD_DeformCarBreakWall(*this);

	LD_EVENTOBJECT_DESC TempDesc{};
	if (nullptr == pArg)
	{
		TempDesc.strObjectName = OBJECT_NAME;
		TempDesc.strKind = OBJECT_NAME;
		TempDesc.eCategory = LD_CATEGORY::GIMMICK;
		TempDesc.iModelProtoLevel = m_iPrototypeLevel;
		TempDesc.eModelType = MODEL::ANIM;
		TempDesc.wstrModelProtoTag = MODEL_PROTO_TAG;
		TempDesc.bUseCollMesh = false;
		TempDesc.strAnimEventFile.clear();

		for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
			TempDesc.strAnimNames[i] = DEFORM_CAR_BREAK_WALL_ANIM_NAMES[i];

		pArg = &TempDesc;
	}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_DeformCarBreakWall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_DeformCarBreakWall::Free()
{
	__super::Free();
}

NS_END
