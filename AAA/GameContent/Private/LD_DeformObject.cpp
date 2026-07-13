#include "LD_DeformObject.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "Geometry_Utils.h"

namespace
{
	// 회전 & 빨려들어가는 물리 관련
	static constexpr _float s_fPullInitSpeed = { 4.f };
	static constexpr _float s_fPullAccel = { 18.f };
	static constexpr _float s_fAcquireDistance = { 1.2f };
	static constexpr _float s_fAlignRotSpeedDegree = { 72.f };
	static constexpr _float s_fPullTargetFwd = { 1.8f };
	static constexpr _float s_fReleaseSpeed = { 20.f };
	static constexpr _float s_fReleaseFwd = { -5.f };

	static constexpr _float s_fGravity = { -45.f };
	static constexpr _float s_fMaxFallSpeed = { -15.f };
	static constexpr _float s_fGroundProbeDistance = { 0.02f };
	static constexpr _float s_fMinSweepHalfExtent = { 0.05f };

	static constexpr const _char* ANIM_WAIT = { "Wait" };
	static constexpr const _char* ANIM_SHAKE = { "Shake" };
	static constexpr const _char* ANIM_FALL = { "Fall" };
	static constexpr const _char* ANIM_LANDING = { "Landing" };

	static constexpr _float s_fAnimSpeed = { 1.f };

	struct LD_DEFORMOBJECT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		DEFORM_TYPE eDeformType;
		DEFORM_OBJECT_KIND eKind;
		_float fInteractionRadius;
	};

	static const LD_DEFORMOBJECT_CATALOG g_DeformObjectCatalog[] =
	{
		{ L"DeformCar", L"Proto_Component_Model_DeformCar", "../../Resources/Map/Gimmick/Anim/DeformCar/DeformCar.ysh", MODEL::ANIM, DEFORM_TYPE::CAR, DEFORM_OBJECT_KIND::MOBILE, 6.5f },
		//{ L"DeformCylinder", L"Proto_Component_Model_DeformCylinder", "../../Resources/Map/Gimmick/Anim/DeformCylinder/DeformCylinder.ysh", MODEL::ANIM, DEFORM_TYPE::CYLINDER, DEFORM_OBJECT_KIND::MOBILE, 0.f},
		//{ L"DeformCoaster", L"Proto_Component_Model_DeformCoaster", "../../Resources/Map/Gimmick/Anim/DeformCoaster/DeformCoaster.ysh", MODEL::ANIM, DEFORM_TYPE::COASTER, DEFORM_OBJECT_KIND::FIXED, 0.f },
	};

	const LD_DEFORMOBJECT_CATALOG* Find_DeformObjectCatalog(const _wstring& strObjectName)
	{
		for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, strObjectName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

	_bool Is_SupportedCatalog(const LD_DEFORMOBJECT_CATALOG& Entry)
	{
		return nullptr != Entry.pModelProtoTag
			&& nullptr != Entry.pModelPath
			&& MODEL::END != Entry.eModelType
			&& DEFORM_TYPE::NONE != Entry.eDeformType;
	}
}

NS_BEGIN(Client)

CLD_DeformObject::CLD_DeformObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_DeformObject::CLD_DeformObject(const CLD_DeformObject& Prototype)
	: CLD_EventObject(Prototype)
	, m_tDeformObjectDesc(Prototype.m_tDeformObjectDesc)
	, m_bAvailable(Prototype.m_bAvailable)
	, m_eState(Prototype.m_eState)
	, m_eKind(Prototype.m_eKind)
{
}

HRESULT CLD_DeformObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tDeformObjectDesc = *static_cast<const LD_DEFORMOBJECT_DESC*>(pArg);

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(m_tDeformObjectDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;

	m_eKind = pCatalog->eKind;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	const LD_ANIM_PLAY_DESC AnimDescs[] =
	{
			{ ANIM_WAIT, true, s_fAnimSpeed },
			{ ANIM_SHAKE, true, s_fAnimSpeed },
			{ ANIM_FALL, true, s_fAnimSpeed },
			{ ANIM_LANDING, false, s_fAnimSpeed },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	Play_Anim(ANIM_WAIT);

	m_bUseShadow = true;

	return S_OK;
}

HRESULT CLD_DeformObject::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(m_tDeformObjectDesc.strObjectName);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return E_FAIL;

	if (m_tDeformObjectDesc.wstrModelProtoTag != pCatalog->pModelProtoTag
		|| m_tDeformObjectDesc.eModelType != pCatalog->eModelType
		|| m_tDeformObjectDesc.eDeformType != pCatalog->eDeformType)
		return E_FAIL;

	if (m_tDeformObjectDesc.fInteractionRadius < 0.f)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	if (!m_tDeformObjectDesc.bUseCollMesh || nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_DeformObject::Update(_float fTimeDelta)
{
	switch (m_eState)
	{
	case DEFORM_OBJECT_STATE::IDLE:
	{
		if (!m_bAvailable)
			return;

		__super::Update(fTimeDelta);
		break;
	}
	case DEFORM_OBJECT_STATE::CAPTURED:
	{
		Update_Captured(fTimeDelta);

		if (DEFORM_OBJECT_STATE::CAPTURED == m_eState)
			__super::Update(fTimeDelta);

		break;
	}
	case DEFORM_OBJECT_STATE::ACQUIRED:
		break;

	case DEFORM_OBJECT_STATE::FALLING:
	{
		Update_Falling(fTimeDelta);

		if (DEFORM_OBJECT_STATE::FALLING == m_eState)
			__super::Update(fTimeDelta);

		break;
	}
	case DEFORM_OBJECT_STATE::LANDING:
	{
		__super::Update(fTimeDelta);

		if (m_bAnimationActive)
			break;

		m_bAvailable = true;
		Set_TriggerEnabled(true);
		m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
		Change_State(DEFORM_OBJECT_STATE::IDLE);
		break;
	}
	}
}

void CLD_DeformObject::Late_Update(_float fTimeDelta)
{
	if (m_bAvailable && m_pTrigger->Is_Enabled())
	{
		m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

void CLD_DeformObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_DeformObject::Register_LevelDesignSpecs()
{
	for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
	{
		if (!Is_SupportedCatalog(Entry))
			continue;

		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = LAYER_TAG;
		Spec.eCategory = LD_CATEGORY::GIMMICK;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;

		_float4x4 PreTransformMatrix{};
		XMStoreFloat4x4(&PreTransformMatrix, XMMatrixRotationY(XMConvertToRadians(180.f)));

		Spec.ModelRequirements = { { Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType, true, PreTransformMatrix }, };

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLD_DeformObject::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return false;

	if (Spec.strObjectName != pCatalog->pObjectName
		|| Spec.strPrototypeTag != PROTOTYPE_TAG
		|| Spec.strLayerTag != LAYER_TAG
		|| Spec.eCategory != LD_CATEGORY::GIMMICK
		|| Spec.wstrModelProtoTag != pCatalog->pModelProtoTag
		|| Spec.eModelType != pCatalog->eModelType)
		return false;

	LD_DEFORMOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = pCatalog->eModelType;
	Desc.wstrModelProtoTag = pCatalog->pModelProtoTag;
	Desc.bUseCollMesh = true;
	Desc.strAnimEventFile.clear();
	Desc.eDeformType = pCatalog->eDeformType;
	Desc.fInteractionRadius = pCatalog->fInteractionRadius;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_DeformObject::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_DeformObject::Create(pDevice, pContext);
}

HRESULT CLD_DeformObject::On_DeformAcquired()
{
	if (!m_bAvailable)
		return S_FALSE;

	m_bAvailable = false;
	Set_TriggerEnabled(false);
	Release_RigidStatic();
	Set_Active(false);

	return S_OK;
}

HRESULT CLD_DeformObject::On_DeformReleased(const _float3& vWorldPosition, const _float3& vTargetPosition)
{
	if (m_bAvailable)
		return S_FALSE;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vWorldPosition), 1.f));
	m_vReleaseTargetPosition = vTargetPosition;
	m_fVerticalVelocity = 0.f;

	Set_Active(true);
	Set_TriggerEnabled(false);
	Change_State(DEFORM_OBJECT_STATE::FALLING);

	return S_OK;
}

#pragma region Deformable
_bool CLD_DeformObject::Request_Deform(const _float4x4* AnchorWorld)
{
	if (!m_bAvailable || DEFORM_OBJECT_STATE::IDLE != m_eState)
		return false;

	if (DEFORM_OBJECT_KIND::MOBILE != m_eKind)
		return false;

	Change_State(DEFORM_OBJECT_STATE::CAPTURED);
	m_AnchorWorld = *AnchorWorld;
	m_bAlignDone = false;

	Set_TriggerEnabled(false);
	Release_RigidStatic();

	m_fPullSpeed = s_fPullInitSpeed;
	m_pTransformCom->Set_RotationPerSec(s_fAlignRotSpeedDegree);

	return true;
}

void CLD_DeformObject::End_Deform(const _float4x4* AnchorWorld)
{
	if (DEFORM_OBJECT_STATE::ACQUIRED != m_eState)
		return;

	_matrix Anchor = XMLoadFloat4x4(AnchorWorld);

	_vector vLook = XMVectorSetY(Anchor.r[2], 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= FLT_EPSILON)
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	vLook = XMVector3Normalize(vLook);

	const _vector vReleaseStartPosition = XMVectorSetW(Anchor.r[3], 1.f);
	const _vector vReleaseTargetPosition = XMVectorSetW(vReleaseStartPosition + vLook * s_fReleaseFwd, 1.f);

	_float3 vStartPosition{};
	_float3 vTargetPosition{};
	XMStoreFloat3(&vStartPosition, vReleaseStartPosition);
	XMStoreFloat3(&vTargetPosition, vReleaseTargetPosition);

	if (S_OK != On_DeformReleased(vStartPosition, vTargetPosition))
		return;

	m_pTransformCom->LookAt(vReleaseStartPosition + vLook);
}
#pragma endregion

void CLD_DeformObject::Update_Captured(_float fTimeDelta)
{
	_matrix AnchorWorld = XMLoadFloat4x4(&m_AnchorWorld);
	_vector vAnchorPos = AnchorWorld.r[3];
	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);

	if (!m_bAlignDone)
	{
		const _vector vAlignTarget = vSelf - (vAnchorPos - vSelf);
		m_bAlignDone = m_pTransformCom->LookAt_Smooth(vAlignTarget, fTimeDelta);
		return;
	}

	_vector vLook = XMVectorSetY(AnchorWorld.r[2], 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= FLT_EPSILON)
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	vLook = XMVector3Normalize(vLook);

	_vector vTarget = vAnchorPos + vLook * s_fPullTargetFwd;
	vTarget = XMVectorSetY(vTarget, XMVectorGetY(vSelf));
	vTarget = XMVectorSetW(vTarget, 1.f);

	_vector vDir = vTarget - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= s_fAcquireDistance)
	{
		Change_State(DEFORM_OBJECT_STATE::ACQUIRED);
		On_DeformAcquired();

		DEFORM_ACQUIRED_EVENT Payload{};
		Payload.eType = Get_DeformType();
		Payload.pSource = this;
		m_pGameInstance_Proxy->Publish(EventTag::Deform_Acquired, &Payload);

		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	m_pTransformCom->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vDir) * fMove);
}

void CLD_DeformObject::Update_Falling(_float fTimeDelta)
{
	m_fVerticalVelocity = max(m_fVerticalVelocity + s_fGravity * fTimeDelta, s_fMaxFallSpeed);

	const _float fFallDistance = -m_fVerticalVelocity * fTimeDelta;
	if (fFallDistance <= 0.f)
		return;

	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vHorizontalTarget = XMLoadFloat3(&m_vReleaseTargetPosition);
	vHorizontalTarget = XMVectorSetY(vHorizontalTarget, XMVectorGetY(vPosition));
	vHorizontalTarget = XMVectorSetW(vHorizontalTarget, 1.f);

	const _vector vHorizontalDirection = vHorizontalTarget - vPosition;
	const _float fHorizontalDistance = XMVectorGetX(XMVector3Length(vHorizontalDirection));

	if (fHorizontalDistance > FLT_EPSILON)
	{
		const _float fHorizontalMoveDistance = min(s_fReleaseSpeed * fTimeDelta, fHorizontalDistance);
		vPosition += XMVector3Normalize(vHorizontalDirection) * fHorizontalMoveDistance;
		m_pTransformCom->Set_State(STATE::POSITION, vPosition);
	}

	const _float3 vScale = m_pTransformCom->Get_Scaled();
	const _float3 vHalfExtents =
	{
			max(m_LocalCollisionBounds.Extents.x * vScale.x, s_fMinSweepHalfExtent),
			max(m_LocalCollisionBounds.Extents.y * vScale.y, s_fMinSweepHalfExtent),
			max(m_LocalCollisionBounds.Extents.z * vScale.z, s_fMinSweepHalfExtent)
	};

	const _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	_float3 vWorldCenter{};
	XMStoreFloat3(&vWorldCenter, XMVector3TransformCoord(XMLoadFloat3(&m_LocalCollisionBounds.Center), WorldMatrix));

	_float4 vRotation{};
	XMStoreFloat4(&vRotation, XMQuaternionRotationMatrix(m_pTransformCom->Get_RotationMatrix()));
	const _float3 vDown = { 0.f, -1.f, 0.f };

	_float fHitDistance = 0.f;

	const _bool bHit = m_pGameInstance_Proxy->Sweep_Box(
		vWorldCenter,
		vHalfExtents,
		vRotation,
		vDown,
		fFallDistance + s_fGroundProbeDistance,
		nullptr,
		&fHitDistance,
		true,
		false);

	_float fMoveDistance = fFallDistance;
	if (bHit)
		fMoveDistance = max(0.f, fHitDistance);

	vPosition = XMVectorSetY(vPosition, XMVectorGetY(vPosition) - fMoveDistance);
	m_pTransformCom->Set_State(STATE::POSITION, vPosition);

	if (!bHit)
		return;

	if (FAILED(Ready_RigidStatic()))
		return;

	m_fVerticalVelocity = 0.f;
	Change_State(DEFORM_OBJECT_STATE::LANDING);
}

HRESULT CLD_DeformObject::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	return Ready_Trigger();
}

void CLD_DeformObject::On_Deserialized()
{
	__super::On_Deserialized();

	if (!m_bAvailable)
		return;

	if (FAILED(Ready_RigidStatic()))
		return;

	m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

HRESULT CLD_DeformObject::Ready_Trigger()
{
	_float3 vMin{};
	_float3 vMax{};
	if (!m_pModelCom->Get_CollisionAABB(&vMin, &vMax))
		return E_FAIL;

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
		return E_FAIL;

	m_LocalCollisionBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);

	const _float fBoundsRadius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_LocalCollisionBounds.Extents)));
	const _float fInteractionRadius = 0.f < m_tDeformObjectDesc.fInteractionRadius ? m_tDeformObjectDesc.fInteractionRadius : fBoundsRadius;

	if (fInteractionRadius <= 0.f)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = m_LocalCollisionBounds.Center;
	ColliderDesc.fRadius = fInteractionRadius;

	m_pTrigger = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_Trigger"), &ColliderDesc);
	if (nullptr == m_pTrigger)
		return E_FAIL;

	m_pTrigger->Set_OnEnter(
		[this](CCollider* pOther)
		{
			if (pOther->Get_RegisteredGroup() == ETOUI(COLLISION_LAYER::PLAYER_HURT))
				m_bKirbyInTrigger = true;
		});

	m_pTrigger->Set_OnExit(
		[this](CCollider* pOther)
		{
			if (pOther->Get_RegisteredGroup() == ETOUI(COLLISION_LAYER::PLAYER_HURT))
				m_bKirbyInTrigger = false;
		});

	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::DEFORM_OBJECT));

	return S_OK;
}

void CLD_DeformObject::Set_TriggerEnabled(_bool bEnabled)
{
	if (!bEnabled)
		m_bKirbyInTrigger = false;

	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_Enabled(bEnabled);
}

void CLD_DeformObject::Change_State(DEFORM_OBJECT_STATE eState)
{
	if (m_eState == eState)
		return;

	m_eState = eState;

	switch (m_eState)
	{
	case DEFORM_OBJECT_STATE::IDLE:
		Play_Anim(ANIM_WAIT);
		break;

	case DEFORM_OBJECT_STATE::CAPTURED:
		Play_Anim(ANIM_SHAKE);
		break;

	case DEFORM_OBJECT_STATE::ACQUIRED:
		m_bAnimationActive = false;
		break;

	case DEFORM_OBJECT_STATE::FALLING:
		Play_Anim(ANIM_FALL);
		break;

	case DEFORM_OBJECT_STATE::LANDING:
		Play_Anim(ANIM_LANDING);
		break;
	}
}

CLD_DeformObject* CLD_DeformObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_DeformObject* pInstance = new CLD_DeformObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_DeformObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_DeformObject::Clone(void* pArg)
{
	CLD_DeformObject* pInstance = new CLD_DeformObject(*this);

	LD_DEFORMOBJECT_DESC TempDesc{};
	if (nullptr == pArg)
	{
		const LD_DEFORMOBJECT_CATALOG* pCatalog = nullptr;
		for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
		{
			if (Is_SupportedCatalog(Entry))
			{
				pCatalog = &Entry;
				break;
			}
		}

		if (nullptr == pCatalog)
		{
			MSG_BOX("Failed to Cloned : CLD_DeformObject");
			Safe_Release(pInstance);
			return nullptr;
		}

		TempDesc.strObjectName = pCatalog->pObjectName;
		TempDesc.strKind = pCatalog->pObjectName;
		TempDesc.eCategory = LD_CATEGORY::GIMMICK;
		TempDesc.iModelProtoLevel = m_iPrototypeLevel;
		TempDesc.eModelType = pCatalog->eModelType;
		TempDesc.wstrModelProtoTag = pCatalog->pModelProtoTag;
		TempDesc.bUseCollMesh = true;
		TempDesc.strAnimEventFile.clear();
		TempDesc.eDeformType = pCatalog->eDeformType;
		TempDesc.fInteractionRadius = pCatalog->fInteractionRadius;

		pArg = &TempDesc;
	}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_DeformObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_DeformObject::Free()
{
	__super::Free();
}

NS_END
