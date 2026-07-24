#include "EnvInteract_KickProp.h"
#include "GameContent_const.h"
#include "GameContent_Events.h"
#include "Effect_Loader.h"

#include "GameInstance_Proxy.h"
#include "Geometry_Utils.h"
#include "Collider.h"
#include "RigIdBody.h"

NS_BEGIN(Client)

CEnvInteract_KickProp::CEnvInteract_KickProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Interact(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvInteract_KickProp::CEnvInteract_KickProp(const CEnvInteract_KickProp& Prototype)
	: CEnvObject_Interact(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvInteract_KickProp::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

void CEnvInteract_KickProp::Late_Update(_float fTimeDelta)
{
	if (!Is_Active())
		return;

	if (m_pGameInstance_Proxy->Is_EditMode())
	{
		__super::Late_Update(fTimeDelta);
		return;
	}

	if (m_bKickPending)
	{
		m_pRigidBodyCom->Set_Enabled(true);
		m_pRigidBodyCom->Set_SceneQueryEnabled(false);   // CCT는 씬 쿼리로 발판을 찾음 - 찬 돌 위에 못 올라타게 제외
		m_pRigidBodyCom->Set_LinearVelocity(XMLoadFloat3(&m_vPendingKickVelocity));

		m_bKickPending = false;
		m_bKicked = true;
	}

	if (m_bKicked)
	{
		Clamp_DynamicVelocity();
		Update_BounceState(fTimeDelta);

		if (!Is_Active())
			return;
	}

	m_pRigidBodyCom->Sync_From_Body();
	Sync_InteractCollider();
	__super::Late_Update(fTimeDelta);
}

HRESULT CEnvInteract_KickProp::Ready_InteractComponents()
{
	if (ENV_INTERACT_TYPE::PHYSICS_PROP != m_tDesc.eInteractType)
		return E_FAIL;

	if (ENV_INTERACT_TYPE::PHYSICS_PROP != m_tDesc.tInteractPreset.eType)
		return E_FAIL;

	if (nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	if (m_pGameInstance_Proxy->Is_EditMode())
		return S_OK;

	if (FAILED(Ready_DynamicActor()))
		return E_FAIL;

	if (m_tDesc.tInteractPreset.bTouchByPlayerBody && FAILED(Ready_InteractCollider()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnvInteract_KickProp::Ready_DynamicActor()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;

	if (m_pGameInstance_Proxy->Is_EditMode())
		return S_OK;

	if (nullptr != m_pRigidBodyCom)
		return E_FAIL;

	const ENV_INTERACT_PRESET& Preset = m_tDesc.tInteractPreset;
	if (!MathUtils::Is_FiniteFloat(Preset.fDensity) || Preset.fDensity <= 0.f)
		return E_FAIL;

	_float3 vSize = m_tDesc.tCollision.vSize;
	if (!GeometryUtils::Has_UsableSize(vSize))
		vSize = Preset.vFallbackSize;

	if (!GeometryUtils::Has_UsableSize(vSize))
		return E_FAIL;

	_vector vWorldScale{};
	_vector vWorldRotation{};
	_vector vWorldPosition{};
	if (!XMMatrixDecompose(&vWorldScale, &vWorldRotation, &vWorldPosition,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())))
	{
		return E_FAIL;
	}

	_float3 vPosition{};
	_float4 qRotation{};
	XMStoreFloat3(&vPosition, vWorldPosition);
	XMStoreFloat4(&qRotation, XMQuaternionNormalize(vWorldRotation));

	physx::PxRigidDynamic* pDynamicActor = nullptr;

	switch (Preset.eShape)
	{
	case ENV_INTERACT_SHAPE::BOX:
	{
		const _float3 vHalfExtents = GeometryUtils::Make_HalfExtentsFromSize(vSize);
		pDynamicActor = m_pGameInstance_Proxy->Create_DynamicBox(
			vPosition, qRotation, vHalfExtents, Preset.fDensity);
		break;
	}

	case ENV_INTERACT_SHAPE::SPHERE:
	{
		_float fRadius = m_tDesc.tCollision.fMapCollRadius;
		if (!MathUtils::Is_FiniteFloat(fRadius) || fRadius <= 0.f)
		{
			const _float3 vHalfExtents = GeometryUtils::Make_HalfExtentsFromSize(vSize);
			fRadius = max(max(vHalfExtents.x, vHalfExtents.y), vHalfExtents.z);
		}

		if (!MathUtils::Is_FiniteFloat(fRadius) || fRadius <= 0.f)
			return E_FAIL;

		pDynamicActor = m_pGameInstance_Proxy->Create_DynamicSphere(
			vPosition, fRadius, Preset.fDensity);
		break;
	}

	case ENV_INTERACT_SHAPE::CAPSULE:
	{
		_float fRadius{};
		_float fHalfHeight{};
		if (!GeometryUtils::Try_Make_YAxisCapsuleFromSize(vSize, &fRadius, &fHalfHeight))
			return E_FAIL;

		pDynamicActor = m_pGameInstance_Proxy->Create_DynamicCapsule(
			vPosition, qRotation, fRadius, fHalfHeight, Preset.fDensity);
		break;
	}

	default:
		return E_FAIL;
	}

	if (nullptr == pDynamicActor)
		return E_FAIL;

	CRigidBody* pRigidBody = CRigidBody::Create(m_pDevice, m_pContext);
	if (nullptr == pRigidBody)
	{
		m_pGameInstance_Proxy->Remove_DynamicActor(pDynamicActor);
		return E_FAIL;
	}

	pRigidBody->Set_Kinematic(false);

	m_pRigidBodyCom = Add_Component<CRigidBody>(TEXT("Com_RigidBody"), pRigidBody);
	if (nullptr == m_pRigidBodyCom)
	{
		Safe_Release(pRigidBody);
		m_pGameInstance_Proxy->Remove_DynamicActor(pDynamicActor);
		return E_FAIL;
	}

	m_pRigidBodyCom->Set_Body(m_pTransformCom, pDynamicActor);
	m_pRigidBodyCom->Set_Enabled(false);
	return S_OK;
}

HRESULT CEnvInteract_KickProp::Ready_InteractCollider()
{
	if (nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	if (m_pGameInstance_Proxy->Is_EditMode())
		return S_OK;

	if (nullptr != m_pInteractCollider)
		return E_FAIL;

	const ENV_INTERACT_PRESET& Preset = m_tDesc.tInteractPreset;

	_float3 vSize = m_tDesc.tCollision.vSize;
	if (!GeometryUtils::Has_UsableSize(vSize))
		vSize = Preset.vFallbackSize;

	if (!GeometryUtils::Has_UsableSize(vSize))
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };

	switch (Preset.eShape)
	{
	case ENV_INTERACT_SHAPE::BOX:
		ColliderDesc.vSize = GeometryUtils::Make_AbsSize(vSize);
		m_pInteractCollider = Add_Component<CCollider>(
			Collider_OBB.iLevelID,
			Collider_OBB.szProtoTag,
			TEXT("Com_InteractCollider"),
			&ColliderDesc);
		break;

	case ENV_INTERACT_SHAPE::SPHERE:
	{
		_float fRadius = m_tDesc.tCollision.fMapCollRadius;
		if (!MathUtils::Is_FiniteFloat(fRadius) || fRadius <= 0.f)
		{
			const _float3 vHalfExtents = GeometryUtils::Make_HalfExtentsFromSize(vSize);
			fRadius = max(max(vHalfExtents.x, vHalfExtents.y), vHalfExtents.z);
		}

		if (!MathUtils::Is_FiniteFloat(fRadius) || fRadius <= 0.f)
			return E_FAIL;

		ColliderDesc.fRadius = fRadius;
		m_pInteractCollider = Add_Component<CCollider>(
			Collider_Sphere.iLevelID,
			Collider_Sphere.szProtoTag,
			TEXT("Com_InteractCollider"),
			&ColliderDesc);
		break;
	}

	case ENV_INTERACT_SHAPE::CAPSULE:
	{
		_float fRadius{};
		_float fHalfHeight{};
		if (!GeometryUtils::Try_Make_YAxisCapsuleFromSize(vSize, &fRadius, &fHalfHeight))
			return E_FAIL;

		ColliderDesc.vCenter = { 0.f, -(fRadius + fHalfHeight), 0.f };
		ColliderDesc.fRadius = fRadius;
		ColliderDesc.fHeight = fHalfHeight * 2.f;

		m_pInteractCollider = Add_Component<CCollider>(
			Collider_Capsule.iLevelID,
			Collider_Capsule.szProtoTag,
			TEXT("Com_InteractCollider"),
			&ColliderDesc);
		break;
	}

	default:
		return E_FAIL;
	}

	if (nullptr == m_pInteractCollider)
		return E_FAIL;

	m_pInteractCollider->Set_OnEnter([this](CCollider* pOther)
		{
			Handle_InteractColliderEnter(pOther);
		});

	Sync_InteractCollider();
	m_pGameInstance_Proxy->Register_Collider(m_pInteractCollider, ETOUI(COLLISION_LAYER::ENV_INTERACT));

	return S_OK;
}

void CEnvInteract_KickProp::Sync_InteractCollider()
{
	if (nullptr == m_pInteractCollider || !m_pInteractCollider->Is_Enabled())
		return;

	_vector vWorldScale{};
	_vector vWorldRotation{};
	_vector vWorldPosition{};
	if (!XMMatrixDecompose(&vWorldScale, &vWorldRotation, &vWorldPosition,
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())))
	{
		m_pInteractCollider->Set_Enabled(false);
		return;
	}

	const _matrix InteractColliderWorld =
		XMMatrixRotationQuaternion(XMQuaternionNormalize(vWorldRotation))
		* XMMatrixTranslationFromVector(vWorldPosition);

	m_pInteractCollider->Update(InteractColliderWorld);

#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractCollider);
#endif
}

void CEnvInteract_KickProp::Handle_InteractColliderEnter(CCollider* pOther)
{
	if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
		return;

	Kick_FromPlayer(pOther->Get_Owner());
}

void CEnvInteract_KickProp::Kick_FromPlayer(CGameObject* pPlayer)
{
	if (m_bKicked || m_bKickPending)
		return;

	CTransform* pPlayerTransform = pPlayer->Get_Transform();
	const ENV_INTERACT_PRESET& Preset = m_tDesc.tInteractPreset;

	// 발로 찬 느낌: 플레이어 → 프롭 수평 방향. 거의 겹쳐 있으면 플레이어 정면으로 대체
	_vector vKickDirection = XMVectorSetY(
		m_pTransformCom->Get_State(STATE::POSITION) - pPlayerTransform->Get_State(STATE::POSITION), 0.f);

	if (XMVectorGetX(XMVector3LengthSq(vKickDirection)) < Helper::fEpsilon)
		vKickDirection = XMVectorSetY(pPlayerTransform->Get_State(STATE::LOOK), 0.f);

	vKickDirection = XMVector3Normalize(vKickDirection);

	const _float fYaw = XMConvertToRadians(m_pGameInstance_Proxy->RandomFloat(-s_fKickYawDegree, s_fKickYawDegree));
	vKickDirection = XMVector3TransformNormal(vKickDirection, XMMatrixRotationY(fYaw));

	_vector vKickVelocity = XMVectorScale(vKickDirection, Preset.fKickPower);
	vKickVelocity = XMVectorSetY(vKickVelocity, Preset.fUpImpulse);

	XMStoreFloat3(&m_vPendingKickVelocity, vKickVelocity);
	m_bKickPending = true;
}

void CEnvInteract_KickProp::Update_BounceState(_float fTimeDelta)
{
	const ENV_INTERACT_PRESET& Preset = m_tDesc.tInteractPreset;

	m_fKickedElapsed += fTimeDelta;
	if (m_fKickedElapsed >= s_fMaxKickLifeSecond)
	{
		Deactivate();
		return;
	}

	_vector vVelocity = m_pRigidBodyCom->Get_LinearVelocity();
	const _float fVerticalVelocity = XMVectorGetY(vVelocity);

	if (m_fPreviousVerticalVelocity < 0.f && fVerticalVelocity >= 0.f)
	{
		++m_iBounceCount;

		if (m_iBounceCount >= s_iDisappearBounceCount)
		{
			if (Preset.bGrantReward
				&& m_pGameInstance_Proxy->RandomInt(1, 100) <= static_cast<_int>(Preset.iRewardChancePercent))
			{
				KIRBY_POINTSTAR_GAINED_DESC Desc{};
				Desc.iAmount = static_cast<_uint>(Preset.iPointStarAmount);
				m_pGameInstance_Proxy->Publish(EventTag::Kirby_PointStarGained, &Desc);
			}

			m_pRigidBodyCom->Sync_From_Body();

			_float3 vEffectPosition{};
			XMStoreFloat3(&vEffectPosition, m_pTransformCom->Get_State(STATE::POSITION));
			CEffect_Loader::GetInstance()->Spawn(L"VanishEffect", Get_LevelIndex(), vEffectPosition);

			Deactivate();
			return;
		}

		// 튈 때 수평 방향을 살짝 틀어서 점프하듯 다른 쪽으로 뜀
		const _float fYaw = XMConvertToRadians(m_pGameInstance_Proxy->RandomFloat(-s_fBounceYawDegree, s_fBounceYawDegree));
		_vector vHorizontal = XMVector3TransformNormal(XMVectorSetY(vVelocity, 0.f), XMMatrixRotationY(fYaw));
		vHorizontal = XMVectorScale(vHorizontal, Preset.fBounceFriction);

		const _float fBounceUp = -m_fPreviousVerticalVelocity * Preset.fBounceRestitution;

		m_pRigidBodyCom->Set_LinearVelocity(XMVectorSetY(vHorizontal, fBounceUp));
		m_fPreviousVerticalVelocity = fBounceUp;
		return;
	}

	// 씬 중력 -9.81은 커비 스케일(-45) 대비 둥둥 뜸 → 부족분만 매 프레임 보충
	const _float fFallSpeed = fVerticalVelocity + (s_fGravity - s_fPhysXSceneGravity) * fTimeDelta;
	m_pRigidBodyCom->Set_LinearVelocity(XMVectorSetY(vVelocity, fFallSpeed));
	m_fPreviousVerticalVelocity = fFallSpeed;
}

void CEnvInteract_KickProp::Deactivate()
{
	m_pRigidBodyCom->Set_Enabled(false);
	m_pInteractCollider->Set_Enabled(false);
	Set_Active(false);
}

void CEnvInteract_KickProp::Clamp_DynamicVelocity()
{
	const _float fMaxSpeed = m_tDesc.tInteractPreset.fMaxSpeed;
	const _vector vVelocity = m_pRigidBodyCom->Get_LinearVelocity();
	const _float fSpeedSq = XMVectorGetX(XMVector3LengthSq(vVelocity));

	if (fSpeedSq <= fMaxSpeed * fMaxSpeed)
		return;

	m_pRigidBodyCom->Set_LinearVelocity(XMVectorScale(XMVector3Normalize(vVelocity), fMaxSpeed));
}

CEnvInteract_KickProp* CEnvInteract_KickProp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvInteract_KickProp* pInstance = new CEnvInteract_KickProp(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvInteract_KickProp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvInteract_KickProp::Clone(void* pArg)
{
	CEnvInteract_KickProp* pInstance = new CEnvInteract_KickProp(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvInteract_KickProp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvInteract_KickProp::Free()
{
	if (nullptr != m_pInteractCollider)
	{
		m_pInteractCollider->Set_Enabled(false);
		m_pInteractCollider->Clear_Callbacks();

		if (nullptr != m_pGameInstance_Proxy)
		{
			m_pGameInstance_Proxy->Immediate_Unregister(m_pInteractCollider, ETOUI(COLLISION_LAYER::ENV_INTERACT));
		}

		m_pInteractCollider->Mark_Unregistered();
	}

	m_pInteractCollider = nullptr;
	m_pRigidBodyCom = nullptr;

	__super::Free();
}

NS_END