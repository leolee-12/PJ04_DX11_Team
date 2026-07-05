#include "LD_DeformCarBreakWall.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _char* DEFORM_CAR_BREAK_WALL_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/DeformCarBreakWall/DeformCarBreakWall.ysh";
	inline constexpr const _char* DEFORM_CAR_BREAK_WALL_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { "DeformCarGetFirst", "DeformCarGetEnd", "", "" };
	inline constexpr const _uint DEFORM_CAR_BREAK_WALL_MESH_COUNT = 31u;
	inline constexpr const _uint DEFORM_CAR_BREAK_WALL_COLLIMESH_INDEX = 10u;

	inline constexpr const _uint DISABLE_MESH_INDEX[] = { 1 };
	inline constexpr const _uint ON_TO_OFF_MESH_INDEX[] = { 0,2,9,10 };
	inline constexpr const _uint OFF_TO_ON_MESH_INDEX[] = { 3,4,5,6,7,8,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 };
	//inline constexpr const _uint STAY_MESH_INDEX[] = { 9,10 };
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

	if (nullptr == m_pBoostTrigger || !m_bBoostTriggerRegistered)
		return E_FAIL;

	return S_OK;
}

void CLD_DeformCarBreakWall::Update(_float fTimeDelta)
{
	const _bool bAnimationWasActive = m_bAnimationActive;

	__super::Update(fTimeDelta);

	if (bAnimationWasActive
		&& !m_bAnimationActive
		&& STATE::BREAKING == m_eState)
	{
		m_eState = STATE::BROKEN;
	}
}

void CLD_DeformCarBreakWall::Late_Update(_float fTimeDelta)
{
	if (STATE::IDLE == m_eState && nullptr != m_pBoostTrigger && m_pBoostTrigger->Is_Enabled())
	{
		m_pBoostTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pBoostTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
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

_bool CLD_DeformCarBreakWall::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec,
	LD_OBJECT_ENTRY* pOutEntry)
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

	if (FAILED(Ready_DeformCarBreakWall()))
		return E_FAIL;

	if (FAILED(Ready_WallRigidStatic()))
		return E_FAIL;

	if (FAILED(Ready_BoostTrigger()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_DeformCarBreakWall::Ready_DeformCarBreakWall()
{
	m_eState = STATE::IDLE;

	const _int iAnimationIndex = m_pModelCom->Get_AnimationIndex(DEFORM_CAR_BREAK_WALL_ANIM_NAMES[0]);
	if (iAnimationIndex < 0)
		return E_FAIL;

	m_pModelCom->Set_AnimationIndex(static_cast<_uint>(iAnimationIndex), false, true, 0.f);
	m_pModelCom->Seek_Animation(0.f);

	if (DEFORM_CAR_BREAK_WALL_MESH_COUNT != static_cast<_uint>(m_pModelCom->Get_NumMeshes()))
		return E_FAIL;

	for (_uint iMeshIndex : DISABLE_MESH_INDEX)
		Set_MeshVisible(iMeshIndex, false);

	for (_uint iMeshIndex : ON_TO_OFF_MESH_INDEX)
		Set_MeshVisible(iMeshIndex, true);

	for (_uint iMeshIndex : OFF_TO_ON_MESH_INDEX)
		Set_MeshVisible(iMeshIndex, false);

	//for (_uint iMeshIndex : STAY_MESH_INDEX)
	//	Set_MeshVisible(iMeshIndex, true);

	return S_OK;
}

HRESULT CLD_DeformCarBreakWall::Ready_WallRigidStatic()
{
	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(DEFORM_CAR_BREAK_WALL_COLLIMESH_INDEX, &vMin, &vMax);

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

	if (vLocalHalfExtents.x <= 0.f || vLocalHalfExtents.y <= 0.f || vLocalHalfExtents.z <= 0.f)
		return E_FAIL;

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(
		vLocalCenter, vLocalHalfExtents, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	return nullptr != m_pRigidStatic ? S_OK : E_FAIL;
}

void CLD_DeformCarBreakWall::On_Event()
{
	if (STATE::IDLE != m_eState)
		return;

	if (!Play_EventAnimation(0u, false))
		return;

	for (_uint iMeshIndex : ON_TO_OFF_MESH_INDEX)
		Set_MeshVisible(iMeshIndex, false);

	for (_uint iMeshIndex : OFF_TO_ON_MESH_INDEX)
		Set_MeshVisible(iMeshIndex, true);

	m_eState = STATE::BREAKING;

	Release_RigidStatic();
	Unregister_BoostTrigger(false);
}

HRESULT CLD_DeformCarBreakWall::Ready_BoostTrigger()
{
	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(DEFORM_CAR_BREAK_WALL_COLLIMESH_INDEX, &vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	constexpr _float fPadding = 2.f;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	ColliderDesc.vSize = { (vMax.x - vMin.x) + fPadding * 2.f, (vMax.y - vMin.y) + fPadding * 2.f, (vMax.z - vMin.z) + fPadding * 2.f };

	if (ColliderDesc.vSize.x <= 0.f || ColliderDesc.vSize.y <= 0.f || ColliderDesc.vSize.z <= 0.f)
		return E_FAIL;

	m_pBoostTrigger = Add_Component<CCollider>(Collider_OBB.iLevelID, Collider_OBB.szProtoTag, TEXT("Com_BoostTrigger"),
		&ColliderDesc);

	if (nullptr == m_pBoostTrigger)
		return E_FAIL;

	SetUp_BoostTriggerCallback();

	m_pGameInstance_Proxy->Register_Collider(m_pBoostTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));
	m_bBoostTriggerRegistered = true;

	return S_OK;
}

void CLD_DeformCarBreakWall::SetUp_BoostTriggerCallback()
{
	if (nullptr == m_pBoostTrigger)
		return;

	m_pBoostTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_BoostTrigger(pOther); });
}

void CLD_DeformCarBreakWall::Handle_BoostTrigger(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::CAR_BOOST) != pOther->Get_RegisteredGroup())
		return;

	On_Event();
}

void CLD_DeformCarBreakWall::Unregister_BoostTrigger(_bool bImmediate)
{
	if (nullptr == m_pBoostTrigger)
		return;

	m_pBoostTrigger->Set_Enabled(false);

	if (!m_bBoostTriggerRegistered)
		return;

	const _uint iGroup = ETOUI(COLLISION_LAYER::ENV_TRIGGER);

	if (bImmediate)
		m_pGameInstance_Proxy->Immediate_Unregister(m_pBoostTrigger, iGroup);
	else
		m_pGameInstance_Proxy->Request_Unregister(m_pBoostTrigger, iGroup);

	m_bBoostTriggerRegistered = false;
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

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_DeformCarBreakWall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_DeformCarBreakWall::Free()
{
	Unregister_BoostTrigger(true);

	__super::Free();
}

NS_END