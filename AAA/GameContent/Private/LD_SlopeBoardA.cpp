#include "LD_SlopeBoardA.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	inline constexpr const _char* SLOPEBOARD_A_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardA.ysh";
	inline constexpr const _char* ANIM_LANDBACK = "LandBack";
	inline constexpr const _char* ANIM_LANDFRONT = "LandFront";
	inline constexpr const _char* ANIM_LANDSTARTFRONT = "LandStartFront";
	inline constexpr const _char* SLOPEBOARD_A_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { ANIM_LANDBACK, ANIM_LANDFRONT, ANIM_LANDSTARTFRONT, "" };

	inline constexpr const _char* SLOPEBOARD_A_PLATFORM_MESH_NAME = "PlateM__BoardC";
	inline constexpr const _char* SLOPEBOARD_A_PLATFORM_BONE_NAME = "FrontAnimL";

	inline constexpr _float SLOPEBOARD_A_MIN_HALF_EXTENT = 0.01f;
	inline constexpr _float SLOPEBOARD_A_FALLEN_ANGLE_RAD = XM_PI / 3.f + 0.05f;
	inline constexpr _float SLOPEBOARD_A_FALL_VISUAL_DURATION = 1.f;
	inline constexpr _float SLOPEBOARD_A_ANIM_SPEED = 1.5f;

	inline constexpr const _tchar* SLOPEBOARD_FALL_START	= L"GimmickSlopeBoard_SlopeBoardABFallStart.wav";
	inline constexpr const _tchar* SLOPEBOARD_FALL_END		= L"GimmickSlopeBoard_SlopeBoardABFallEnd.wav";

	void Log_SlopeBoardPhysicsWarning(const _string& strMessage)
	{
#ifdef _DEBUG
		OutputDebugStringA((strMessage + "\n").c_str());
#else
		UNREFERENCED_PARAMETER(strMessage);
#endif
	}

	_matrix RemoveScale_FromWorldMatrix(_fmatrix WorldMatrix)
	{
		XMVECTOR vScale{}, vRotation{}, vTranslation{};
		if (!XMMatrixDecompose(&vScale, &vRotation, &vTranslation, WorldMatrix))
			return WorldMatrix;

		return XMMatrixRotationQuaternion(XMQuaternionNormalize(vRotation))
			* XMMatrixTranslationFromVector(vTranslation);
	}

	_matrix Make_FallenLocalMatrix(const _float3& vMin, const _float3& vMax, _float fAngleRad =
		SLOPEBOARD_A_FALLEN_ANGLE_RAD)
	{
		const _float3 vPivot =
		{
			(vMin.x + vMax.x) * 0.5f,
			vMin.y,
			(vMin.z + vMax.z) * 0.5f
		};

		return XMMatrixTranslation(-vPivot.x, -vPivot.y, -vPivot.z)
			* XMMatrixRotationX(fAngleRad)
			* XMMatrixTranslation(vPivot.x, vPivot.y, vPivot.z);
	}

	_float EaseInQuad(_float fRatio)
	{
		fRatio = max(0.f, min(fRatio, 1.f));
		return fRatio * fRatio;
	}
}

NS_BEGIN(Client)

CLD_SlopeBoardA::CLD_SlopeBoardA(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_SlopeBoardA::CLD_SlopeBoardA(const CLD_SlopeBoardA& Prototype)
	: CLD_EventObject(Prototype)
	, m_eState(Prototype.m_eState)
	, m_fElapsed(Prototype.m_fElapsed)
	, m_bEndAnimationPlayed(Prototype.m_bEndAnimationPlayed)
{
}

HRESULT CLD_SlopeBoardA::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	const LD_ANIM_PLAY_DESC AnimDescs[] =
	{
			{ ANIM_LANDBACK, false, SLOPEBOARD_A_ANIM_SPEED },
			{ ANIM_LANDFRONT, false, SLOPEBOARD_A_ANIM_SPEED },
			{ ANIM_LANDSTARTFRONT, false, SLOPEBOARD_A_ANIM_SPEED },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	m_bUseShadow = true;

	return S_OK;
}

HRESULT CLD_SlopeBoardA::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag !=
		MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tEventObjectDesc.bUseCollMesh || !m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
	{
		if (m_tEventObjectDesc.strAnimNames[i] != SLOPEBOARD_A_ANIM_NAMES[i])
			return E_FAIL;
	}

	if (Find_MeshIndex_ByName(SLOPEBOARD_A_PLATFORM_MESH_NAME) < 0)
		return E_FAIL;

	if (nullptr == m_pModelCom->Get_BoneMatrixPtr(SLOPEBOARD_A_PLATFORM_BONE_NAME))
		return E_FAIL;

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	return S_OK;
}

void CLD_SlopeBoardA::Update(_float fTimeDelta)
{
	const _bool bAnimationWasActive = m_bAnimationActive;

	__super::Update(fTimeDelta);

	if (STATE::PLAYING == m_eState)
	{
		m_fElapsed = min(m_fElapsed + fTimeDelta, SLOPEBOARD_A_FALL_VISUAL_DURATION);

		if (!m_bEndAnimationPlayed && m_fElapsed >= SLOPEBOARD_A_FALL_VISUAL_DURATION)
		{
			m_bEndAnimationPlayed = true;

			if (FAILED(Update_ToFallenPose()))
				Log_SlopeBoardPhysicsWarning("[LDSlopeBoardA] fallen platform rigid static pose update failed.");

			m_pGameInstance_Proxy->Play_SFX(SLOPEBOARD_FALL_END, 0.45f);
			Play_Anim(ANIM_LANDBACK);
		}
	}

	if (bAnimationWasActive
		&& !m_bAnimationActive
		&& STATE::PLAYING == m_eState
		&& m_bEndAnimationPlayed)
	{
		m_eState = STATE::BROKEN;
		m_fElapsed = SLOPEBOARD_A_FALL_VISUAL_DURATION;
	}
}

void CLD_SlopeBoardA::Late_Update(_float fTimeDelta)
{
	if (m_pTrigger->Is_Enabled())
	{
		const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr(SLOPEBOARD_A_PLATFORM_BONE_NAME);
		const _matrix TriggerWorld = RemoveScale_FromWorldMatrix(
			XMLoadFloat4x4(pBoneMatrix)
			* XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

		m_pTrigger->Update(TriggerWorld);

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

void CLD_SlopeBoardA::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_SlopeBoardA::Register_LevelDesignSpecs()
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
	Spec.ModelRequirements =
	{
			{ MODEL_PROTO_TAG, SLOPEBOARD_A_MODEL_PATH, MODEL::ANIM, false },
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_SlopeBoardA::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const
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

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = SLOPEBOARD_A_ANIM_NAMES[i];

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_SlopeBoardA::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_SlopeBoardA::Create(pDevice, pContext);
}

HRESULT CLD_SlopeBoardA::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_RenderComponent()))
		return E_FAIL;

	if (FAILED(Ready_PhysicsComponent()))
		return E_FAIL;

	if (FAILED(Ready_Trigger()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_SlopeBoardA::Ready_RenderComponent()
{
	m_eState = STATE::IDLE;
	m_fElapsed = 0.f;
	m_bEndAnimationPlayed = false;

	const _int iPlatformMeshIndex = Find_MeshIndex_ByName(SLOPEBOARD_A_PLATFORM_MESH_NAME);
	if (iPlatformMeshIndex < 0)
		return E_FAIL;

	if (nullptr == m_pModelCom->Get_BoneMatrixPtr(SLOPEBOARD_A_PLATFORM_BONE_NAME))
		return E_FAIL;

	Set_AllMeshesVisible(false);
	Set_MeshVisible(static_cast<_uint>(iPlatformMeshIndex), true);

	return S_OK;
}

HRESULT CLD_SlopeBoardA::Ready_PhysicsComponent()
{
	const _int iMeshIndex = Find_MeshIndex_ByName(SLOPEBOARD_A_PLATFORM_MESH_NAME);
	if (iMeshIndex < 0)
		return E_FAIL;

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(static_cast<_uint>(iMeshIndex), &vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vLocalCenter =
	{
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	_float3 vLocalHalfExtents =
	{
			(vMax.x - vMin.x) * 0.5f,
			(vMax.y - vMin.y) * 0.5f,
			(vMax.z - vMin.z) * 0.5f
	};

	vLocalHalfExtents.x = max(vLocalHalfExtents.x, SLOPEBOARD_A_MIN_HALF_EXTENT);
	vLocalHalfExtents.y = max(vLocalHalfExtents.y, SLOPEBOARD_A_MIN_HALF_EXTENT);
	vLocalHalfExtents.z = max(vLocalHalfExtents.z, SLOPEBOARD_A_MIN_HALF_EXTENT);

	const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr(SLOPEBOARD_A_PLATFORM_BONE_NAME);
	if (nullptr == pBoneMatrix)
		return E_FAIL;

	const _matrix MeshWorld = RemoveScale_FromWorldMatrix(
		XMLoadFloat4x4(pBoneMatrix)
		* XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(vLocalCenter, vLocalHalfExtents, MeshWorld);

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_SlopeBoardA::Update_ToFallenPose()
{
	const _int iMeshIndex = Find_MeshIndex_ByName(SLOPEBOARD_A_PLATFORM_MESH_NAME);
	if (iMeshIndex < 0)
		return E_FAIL;

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(static_cast<_uint>(iMeshIndex), &vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vLocalCenter =
	{
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	const _matrix MeshWorld = RemoveScale_FromWorldMatrix(
		Make_FallenLocalMatrix(vMin, vMax)
		* XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	return m_pGameInstance_Proxy->Refresh_StaticBoxPose(m_pRigidStatic, vLocalCenter, MeshWorld);
}

HRESULT CLD_SlopeBoardA::Ready_Trigger()
{
	const _int iMeshIndex = Find_MeshIndex_ByName(SLOPEBOARD_A_PLATFORM_MESH_NAME);
	if (iMeshIndex < 0)
		return E_FAIL;

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(static_cast<_uint>(iMeshIndex), &vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vCenter =
	{
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};

	const _float3 vSize =
	{
			vMax.x - vMin.x,
			vMax.y - vMin.y,
			vMax.z - vMin.z
	};

	if (vSize.x <= 0.f || vSize.y <= 0.f || vSize.z <= 0.f)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.vSize = vSize;

	m_pTrigger = Add_Component<CCollider>(
		Collider_OBB.iLevelID,
		Collider_OBB.szProtoTag,
		TEXT("Com_Trigger"),
		&ColliderDesc);

	if (nullptr == m_pTrigger)
		return E_FAIL;

	SetUp_Collider_Callback();

	m_pGameInstance_Proxy->Register_Collider(
		m_pTrigger,
		ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_SlopeBoardA::SetUp_Collider_Callback()
{
	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_OnEnter(
		[this](CCollider* pOther)
		{
			Handle_TriggerEnter(pOther);
		});
}

void CLD_SlopeBoardA::Handle_TriggerEnter(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE) != pOther->Get_RegisteredGroup())
		return;

	On_Event();
}

void CLD_SlopeBoardA::On_Event()
{
	if (STATE::IDLE != m_eState)
		return;

	m_pGameInstance_Proxy->Play_SFX(SLOPEBOARD_FALL_START, 0.45f);

	Play_Anim(ANIM_LANDFRONT);

	m_eState = STATE::PLAYING;
	m_fElapsed = 0.f;
	m_bEndAnimationPlayed = false;
	m_pTrigger->Set_Enabled(false);
}

_bool CLD_SlopeBoardA::Should_RenderMesh(_uint iMeshIndex) const
{
	if (!__super::Should_RenderMesh(iMeshIndex))
		return false;

	if (STATE::BROKEN == m_eState)
		return 0u != iMeshIndex;

	return true;
}

HRESULT CLD_SlopeBoardA::Bind_BoneMatrices(_uint iMeshIndex)
{
	if ((STATE::PLAYING != m_eState && STATE::BROKEN != m_eState)
		|| m_pModelCom->Get_MeshName(iMeshIndex) != SLOPEBOARD_A_PLATFORM_MESH_NAME)
	{
		return __super::Bind_BoneMatrices(iMeshIndex);
	}

	_float3 vMin{}, vMax{};
	m_pModelCom->Get_MeshAABB(iMeshIndex, &vMin, &vMax);

	const _float fRatio = m_fElapsed / SLOPEBOARD_A_FALL_VISUAL_DURATION;
	const _float fAngleRad = SLOPEBOARD_A_FALLEN_ANGLE_RAD * EaseInQuad(fRatio);

	const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr(SLOPEBOARD_A_PLATFORM_BONE_NAME);

	_float4x4 BoneMatrices[1] = {};
	XMStoreFloat4x4(
		&BoneMatrices[0],
		Make_FallenLocalMatrix(vMin, vMax, fAngleRad)
		* XMLoadFloat4x4(pBoneMatrix));

	return m_pShaderCom->Bind_Matrices("g_BoneMatrices", BoneMatrices, 1u);
}

CLD_SlopeBoardA* CLD_SlopeBoardA::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_SlopeBoardA* pInstance = new CLD_SlopeBoardA(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_SlopeBoardA");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_SlopeBoardA::Clone(void* pArg)
{
	CLD_SlopeBoardA* pInstance = new CLD_SlopeBoardA(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_SlopeBoardA");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_SlopeBoardA::Free()
{
	__super::Free();
}

NS_END
