#include "LD_SlopeBoardC.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _tchar* TEMP_EVENT_TAG = L"Temp";

	inline constexpr const _char* SLOPEBOARD_C_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardC.ysh";

	inline constexpr const _char* ANIM_CUT1 = "Cut1";
	inline constexpr const _char* SLOPEBOARD_C_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { ANIM_CUT1 };

	inline constexpr _float SLOPEBOARD_C_ANIM_SPEED = 1.5f;
	inline constexpr _float SLOPEBOARD_C_BOX_THICKNESS = 4.f;
	inline constexpr _float SLOPEBOARD_C_STOP_TRACK_FRAME = 450.f;

	inline constexpr _float3 SLOPEBOARD_C_HORIZONTAL_BOX_CENTER = { 258.f, 23.f - SLOPEBOARD_C_BOX_THICKNESS * 0.5f, -1062.5f };
	inline constexpr _float3 SLOPEBOARD_C_HORIZONTAL_BOX_HALF_EXTENTS = { 12.f, SLOPEBOARD_C_BOX_THICKNESS * 0.5f, 53.5f };

	inline constexpr _float3 SLOPEBOARD_C_VERTICAL_BOX_CENTER = { 258.f, 48.5f, -1116.f + SLOPEBOARD_C_BOX_THICKNESS * 0.5f };
	inline constexpr _float3 SLOPEBOARD_C_VERTICAL_BOX_HALF_EXTENTS = { 12.f, 25.5f, SLOPEBOARD_C_BOX_THICKNESS * 0.5f };


	_matrix Make_ModelPreTransformMatrix()
	{
		return XMMatrixRotationY(XMConvertToRadians(180.f));
	}
}

NS_BEGIN(Client)

CLD_SlopeBoardC::CLD_SlopeBoardC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_SlopeBoardC::CLD_SlopeBoardC(const CLD_SlopeBoardC& Prototype)
	: CLD_EventObject(Prototype)
	, m_eState(Prototype.m_eState)
	, m_fEventStopProgress(Prototype.m_fEventStopProgress)
{
}

HRESULT CLD_SlopeBoardC::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tEventObjectDesc.bUseCollMesh || !m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	if (m_tEventObjectDesc.strAnimNames[0] != SLOPEBOARD_C_ANIM_NAMES[0])
		return E_FAIL;

	if (nullptr == Find_AnimPlayDesc(ANIM_CUT1))
		return E_FAIL;

	if (nullptr == m_pHorizontalPhysicsActor || nullptr == m_pVerticalPhysicsActor)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	return S_OK;
}

void CLD_SlopeBoardC::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (STATE::PLAYING == m_eState && m_pModelCom->Get_CurrentAnimProgress() >= m_fEventStopProgress)
	{
		Release_PhysicsBox(&m_pVerticalPhysicsActor);
		m_eState = STATE::PLAYED;
	}

	if (STATE::PLAYED == m_eState && m_pAnimatorCom->Is_Finished())
	{
		KIRBY_POSITION_SYNC_END_DESC desc{};
		desc.eType = KIRBY_POSITION_SYNC_END_REASON::CAR_BRIDGE_END;
		m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncEnd, &desc);
		m_pGameInstance_Proxy->Publish(EventTag::Letterbox_End, nullptr);
		m_pGameInstance_Proxy->Publish(EventTag::FadeOut_Start, nullptr);
	}
}

void CLD_SlopeBoardC::Late_Update(_float fTimeDelta)
{
	if (m_pTrigger->Is_Enabled())
	{
		m_pTrigger->Update(XMMatrixIdentity());

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

void CLD_SlopeBoardC::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_SlopeBoardC::Register_LevelDesignSpecs()
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

	_float4x4 PreTransformMatrix{};
	XMStoreFloat4x4(&PreTransformMatrix, Make_ModelPreTransformMatrix());
	Spec.ModelRequirements = { { MODEL_PROTO_TAG, SLOPEBOARD_C_MODEL_PATH, MODEL::ANIM, false, PreTransformMatrix }, };

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_SlopeBoardC::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const
	LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.eModelType != MODEL::ANIM ||
		Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = false;
	Desc.strAnimEventFile.clear();

	Desc.strAnimNames[0] = SLOPEBOARD_C_ANIM_NAMES[0];

	*pOutEntry = Desc;
	return true;
}

HRESULT CLD_SlopeBoardC::Ready_Events()
{
	Subscribe_Event(TEMP_EVENT_TAG, [this](void*) { On_Event(); });

	return S_OK;
}

HRESULT CLD_SlopeBoardC::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Ready_AnimEvents()))
		return E_FAIL;

	m_MeshVisible.assign(static_cast<size_t>(m_pModelCom->Get_NumMeshes()), true);

	if (FAILED(Ready_PhysicsBoxes()))
		return E_FAIL;

	if (FAILED(Ready_Trigger()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_SlopeBoardC::Ready_RenderComponents()
{
	if (FAILED(__super::Ready_RenderComponents()))
		return E_FAIL;

	m_eState = STATE::IDLE;

	const LD_ANIM_PLAY_DESC AnimDescs[] =
	{
		  { ANIM_CUT1, false, SLOPEBOARD_C_ANIM_SPEED },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	const _int iAnimationIndex = m_pModelCom->Get_AnimationIndex(ANIM_CUT1);
	if (iAnimationIndex < 0)
		return E_FAIL;

	const _float fEventAnimationDuration = m_pModelCom->Get_AnimationDuration(static_cast<_uint>(iAnimationIndex));
	if (fEventAnimationDuration <= 0.f)
		return E_FAIL;

	m_fEventStopProgress = min(SLOPEBOARD_C_STOP_TRACK_FRAME / fEventAnimationDuration, 1.f);

	if (FAILED(Set_AnimPose(ANIM_CUT1, 0.f)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_SlopeBoardC::Ready_PhysicsBox(const _float3& vWorldCenter, const _float3& vWorldHalfExtents, physx::PxRigidStatic** ppOutActor)
{
	if (nullptr == ppOutActor || nullptr != *ppOutActor || nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	if (vWorldHalfExtents.x <= 0.f || vWorldHalfExtents.y <= 0.f || vWorldHalfExtents.z <= 0.f)
		return E_FAIL;

	const _float3 vLocalCenter = { 0.f, 0.f, 0.f };
	const _matrix PhysicsWorld = XMMatrixTranslation(vWorldCenter.x, vWorldCenter.y, vWorldCenter.z);

	*ppOutActor = m_pGameInstance_Proxy->Create_StaticBox(vLocalCenter, vWorldHalfExtents, PhysicsWorld);
	return nullptr != *ppOutActor ? S_OK : E_FAIL;
}

HRESULT CLD_SlopeBoardC::Ready_PhysicsBoxes()
{
	Release_PhysicsBoxes();

	if (FAILED(Ready_PhysicsBox(SLOPEBOARD_C_HORIZONTAL_BOX_CENTER, SLOPEBOARD_C_HORIZONTAL_BOX_HALF_EXTENTS, &m_pHorizontalPhysicsActor)))
		return E_FAIL;

	if (FAILED(Ready_PhysicsBox(SLOPEBOARD_C_VERTICAL_BOX_CENTER, SLOPEBOARD_C_VERTICAL_BOX_HALF_EXTENTS, &m_pVerticalPhysicsActor)))
		return E_FAIL;

	return S_OK;
}

void CLD_SlopeBoardC::Release_PhysicsBox(physx::PxRigidStatic** ppActor)
{
	if (nullptr == ppActor || nullptr == *ppActor)
		return;

	if (nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(*ppActor);

	*ppActor = nullptr;
}

void CLD_SlopeBoardC::Release_PhysicsBoxes()
{
	Release_PhysicsBox(&m_pHorizontalPhysicsActor);
	Release_PhysicsBox(&m_pVerticalPhysicsActor);
}

HRESULT CLD_SlopeBoardC::Ready_Trigger()
{
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = SLOPEBOARD_C_VERTICAL_BOX_CENTER;
	ColliderDesc.vSize = {
		SLOPEBOARD_C_VERTICAL_BOX_HALF_EXTENTS.x * 2.f,
		SLOPEBOARD_C_VERTICAL_BOX_HALF_EXTENTS.y * 2.f,
		SLOPEBOARD_C_VERTICAL_BOX_HALF_EXTENTS.z * 2.f };

	m_pTrigger = Add_Component<CCollider>(Collider_AABB.iLevelID, Collider_AABB.szProtoTag, TEXT("Com_Trigger"), &ColliderDesc);
	if (nullptr == m_pTrigger)
		return E_FAIL;

	SetUp_Collider_Callback();
	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_SlopeBoardC::SetUp_Collider_Callback()
{
	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_TriggerEnter(pOther); });
}

void CLD_SlopeBoardC::Handle_TriggerEnter(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::CAR_BOOST) != pOther->Get_RegisteredGroup())
		return;

	if (STATE::IDLE != m_eState)
		return;

	_float4x4 vAncorWorld{};
	vAncorWorld = *(m_pTransformCom->Get_WorldMatrixPtr());
	m_pGameInstance_Proxy->Publish(TEMP_EVENT_TAG, &vAncorWorld);
}

void CLD_SlopeBoardC::On_Event()
{
	if (STATE::IDLE != m_eState)
		return;

	Play_Anim(ANIM_CUT1);
	m_eState = STATE::PLAYING;

	CUTSCENE_CAMERA_DESC cam{};
	cam.eCam = ECutsceneCam::Cutscene;
	cam.szTrack = L"Bridge_Cut1_camera1";
	cam.pProgress = m_pAnimatorCom;
	cam.pAnchorWorld = m_pTransformCom->Get_WorldMatrixPtr();
	m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);

	KIRBY_POSITION_SYNC_BEGIN_DESC Pos{};
	Pos.fAnimSpeed = SLOPEBOARD_C_ANIM_SPEED;
	Pos.fBlendDuration = 0.f;
	Pos.AnchorWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	Pos.eType = KIRBY_POSITION_SYNC_CONTEXT::CAR_BRIDGE;
	m_pGameInstance_Proxy->Publish(EventTag::Kirby_PositionSyncBegin, &Pos);

	_bool b = { false };
	m_pGameInstance_Proxy->Publish(EventTag::HUD_SetVisible, &b);
	m_pGameInstance_Proxy->Publish(EventTag::Letterbox_Begin, nullptr);

	m_pInteractionCollider->Set_Enabled(false);
	m_pTrigger->Set_Enabled(false);
}

CGameObject* CLD_SlopeBoardC::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_SlopeBoardC::Create(pDevice, pContext);
}

CLD_SlopeBoardC* CLD_SlopeBoardC::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_SlopeBoardC* pInstance = new CLD_SlopeBoardC(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_SlopeBoardC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_SlopeBoardC::Clone(void* pArg)
{
	CLD_SlopeBoardC* pInstance = new CLD_SlopeBoardC(*this);

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

		TempDesc.strAnimNames[0] = SLOPEBOARD_C_ANIM_NAMES[0];

		pArg = &TempDesc;
	}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_SlopeBoardC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_SlopeBoardC::Free()
{
	Release_PhysicsBoxes();

	__super::Free();
}

NS_END
