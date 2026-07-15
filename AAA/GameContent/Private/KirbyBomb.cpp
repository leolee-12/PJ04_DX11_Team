#include "KirbyBomb.h"

#include "GameContent_Const.h"
#include "Controller.h"
#include "Movement_Child.h"

namespace
{
	constexpr _float BOMB_GRAVITY = -45.f;
	constexpr _float BOMB_GROUND_FRICTION = 10.f;
	constexpr _float BOMB_MAX_FALL_SPEED = -45.f;
}

CKirbyBomb::CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb{ pDevice , pContext }
{
}

CKirbyBomb::CKirbyBomb(const CKirbyBomb& Prototype)
	: CProjectile_Bomb(Prototype)
{
}

HRESULT CKirbyBomb::Initialize(void* pArg)
{
	m_fSpeed = 25.f;
	m_fDamage = 2.f;
	m_fKnockback = 4.f;
	m_fHitHeight = 0.01f;
	m_fHitRadius = 0.60f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CKirbyBomb::Launch(const _float3& vPos, const _float3& vDir)
{
	Launch(vPos, vDir, m_fSpeed);
}

void CKirbyBomb::Launch(const _float3& vPos, const _float3& vDir, _float fLaunchSpeed)
{
	Detach();
	m_bAlive = true;
	m_fAccLife = 0.f;
	On_Activated();

	m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), 0.f);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vPos), 1.f));

	_vector vLaunchVelocity = XMLoadFloat3(&vDir);
	if (XMVectorGetX(XMVector3LengthSq(vLaunchVelocity)) > 1e-6f)
		vLaunchVelocity = XMVector3Normalize(vLaunchVelocity);
	else
		vLaunchVelocity = XMVectorZero();

	vLaunchVelocity *= fLaunchSpeed;
	XMStoreFloat3(&m_vVelocity, vLaunchVelocity);

	m_pHitBox->Set_Enabled(true);
	m_pHitBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	m_bFlying = true;
	m_iBounceCount = 0;

	m_pController->Set_Enabled(true);
	m_pController->Set_FootPosition(XMLoadFloat3(&vPos));

	m_pBombMovement->Clear_Forces();
	m_pBombMovement->Set_Velocity(vLaunchVelocity);

	On_Launched();
}

void CKirbyBomb::Update(_float fTimeDelta)
{
	if (!m_bAlive)
		return;

	Update_State(fTimeDelta);
	// 여기서 m_bAlive 꺼질 수 있음

	if (!m_bAlive)
		return;

	if (!m_bCarried && m_bFlying)
		Update_BombMovement(fTimeDelta);

	__super::Update(fTimeDelta);
}

void CKirbyBomb::Tick_Visual(_float fTimeDelta)
{
	Roll_ByBombMovement(fTimeDelta);
	m_pAnimatorCom->Update(fTimeDelta);
}

HRESULT CKirbyBomb::Ready_Movement()
{
	CController::CONTROLLER_DESC ControllerDesc{};
	ControllerDesc.pOwner = this;
	ControllerDesc.fRadius = m_fHitRadius;
	ControllerDesc.fHeight = m_fHitHeight;
	ControllerDesc.vFootPos = { 0.f, 0.f, 0.f };

	m_pController = Add_Component<CController>(TEXT("Com_Controller"),
		CController::Create(m_pDevice, m_pContext));
	if (nullptr == m_pController || FAILED(m_pController->Initialize(&ControllerDesc)))
		return E_FAIL;

	m_pController->Set_Enabled(false);

	m_pBombMovement = Add_Component<CMovement_Child>(TEXT("Com_Movement"),
		CMovement_Child::Create(m_pDevice, m_pContext));
	if (nullptr == m_pBombMovement)
		return E_FAIL;

	m_pBombMovement->Set_Refs(m_pTransformCom, m_pController->Get_Raw());
	m_pBombMovement->Set_Gravity(BOMB_GRAVITY);
	m_pBombMovement->Set_LinearDrag(0.f);
	m_pBombMovement->Set_UseGroundFriction(true);
	m_pBombMovement->Set_GroundFriction(BOMB_GROUND_FRICTION);
	m_pBombMovement->Set_MaxHorizontalSpeed(0.f);
	m_pBombMovement->Set_MaxFallVelocity(BOMB_MAX_FALL_SPEED);

	return S_OK;
}

HRESULT CKirbyBomb::Ready_Visual()
{
	if (FAILED(__super::Ready_Visual()))
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (m_pModelCom == nullptr)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC tDesc{};
	tDesc.pModel = m_pModelCom;
	//tDesc.strDataFile = TEXT("../../Resources/CHJ/Monster/PoppyBrosJr/EnemyBomb/EnemyBomb_AnimEvents.json");
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CKirbyBomb::On_Activated()
{
	m_eState = KIRBYBOMB_STATE::NONE;

		m_pBombMovement->Stop();
		m_pBombMovement->Clear_Forces();

	if (m_bCarried)
		Change_State(KIRBYBOMB_STATE::HELD);
}

void CKirbyBomb::On_Launched()
{
	Change_State(KIRBYBOMB_STATE::THROW);
}

void CKirbyBomb::On_Bounce(_int iCount)
{
	if (iCount == 1)
		Change_State(KIRBYBOMB_STATE::DANGER);
}

void CKirbyBomb::Update_BombMovement(_float fTimeDelta)
{
	if (nullptr == m_pBombMovement || fTimeDelta <= 0.f)
		return;

	const _bool bWasGrounded = m_pBombMovement->Is_Grounded();
	const _bool bGrounded = m_pBombMovement->Update_RigidBody(fTimeDelta);

	// Bounce 한 번만 호출
	if (!bWasGrounded && bGrounded && 0 == m_iBounceCount)
		On_Bounce(++m_iBounceCount);
}

void CKirbyBomb::Roll_ByBombMovement(_float fTimeDelta)
{
	if (m_bCarried || !m_bFlying ||
		nullptr == m_pAnimatorCom || nullptr == m_pBombMovement)
		return;

	const _float3 vVelocity = m_pBombMovement->Get_Velocity();
	_vector vForward = XMVectorSet(vVelocity.x, 0.f, vVelocity.z, 0.f);
	const _float fHorizontalSpeed = XMVectorGetX(XMVector3Length(vForward));

	if (fHorizontalSpeed > 0.01f)
	{
		vForward = XMVector3Normalize(vForward);
		const _vector vAxisWorld = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vForward);
		const _vector vAxisLocal = XMVector3TransformNormal(vAxisWorld, Get_PreRotInverse());
		XMStoreFloat3(&m_vRollAxis, vAxisLocal);
	}

	m_fRollAngle += fHorizontalSpeed * ROLL_DEG_PER_SPEED * fTimeDelta;
	m_fRollAngle = fmodf(m_fRollAngle, 360.f);

	Apply_RollPose();
}

void CKirbyBomb::Kill()
{
	m_pBombMovement->Stop();
	m_pBombMovement->Clear_Forces();

	__super::Kill();
}

void CKirbyBomb::Change_State(KIRBYBOMB_STATE eNext)
{
	if (m_eState == eNext)
		return;

	Exit_State(m_eState);

	m_eState = eNext;

	Enter_State(eNext);
}

void CKirbyBomb::Enter_State(KIRBYBOMB_STATE eState)
{
	switch (eState)
	{
	case KIRBYBOMB_STATE::HELD:
	{
		Reset_BombVisual();
		Play_BodyAnim(ANIM_FUSE, false);
		Start_Fuse(0.25f);		// Overlay
		Pause_Fuse();			// Overlay Pause
		Update_Socket();
		Spawn_FuseFx();
		break;
	}
	case KIRBYBOMB_STATE::THROW:
	{
		Resume_Fuse(); // Overlay 다시 재생
		break;
	}
	case KIRBYBOMB_STATE::DANGER:
	{
		Play_BodyAnim(ANIM_DANGER, true, 2.f, false);
		break;
	}
	case KIRBYBOMB_STATE::EXPLODEPRE:
	{
		m_iExplodeAniPlayCount = s_iMaxExplodeAniPlayCount;
		Play_BodyAnim("ExplodePre", false, 2.f, true);
		--m_iExplodeAniPlayCount;

		Pause_Fuse();
		break;
	}
	}
}

void CKirbyBomb::Update_State(_float fTimeDelta)
{
	switch (m_eState)
	{
	case KIRBYBOMB_STATE::HELD:
	{
		break;
	}
	case KIRBYBOMB_STATE::THROW:
	{
		break;
	}
	case KIRBYBOMB_STATE::DANGER:
	{
		constexpr _int iSlot = 1;
		m_fBurnRatio = m_pAnimatorCom->Get_LayerProgress(iSlot);

		if (m_fBurnRatio >= 1.f)
			Change_State(KIRBYBOMB_STATE::EXPLODEPRE);
		break;
	}
	case KIRBYBOMB_STATE::EXPLODEPRE:
	{
		if (m_pAnimatorCom->Is_Finished())
		{
			if (m_iExplodeAniPlayCount <= 0)
			{
				Bomb_Explode();
				return;
			}

			Play_BodyAnim("ExplodePre", false, 2.f, true);
			--m_iExplodeAniPlayCount;
		}

		const _int iCompletedCount = s_iMaxExplodeAniPlayCount - m_iExplodeAniPlayCount;

		const _float fCurAnimRatio = m_pAnimatorCom->Get_Progress();

		_float fTotalRatio = (static_cast<_float>(iCompletedCount) + fCurAnimRatio) /
			static_cast<_float>(s_iMaxExplodeAniPlayCount);

		Helper::FloatClamp(fTotalRatio, 0.f, 1.f);
		m_vGlow.x = fTotalRatio / 2.f;

		break;
	}
	}
}

void CKirbyBomb::Exit_State(KIRBYBOMB_STATE eState)
{
	switch (m_eState)
	{
	case KIRBYBOMB_STATE::HELD:
	{
		break;
	}
	case KIRBYBOMB_STATE::THROW:
	{
		break;
	}
	case KIRBYBOMB_STATE::DANGER:
	{
		break;
	}
	case KIRBYBOMB_STATE::EXPLODEPRE:
	{
		break;
	}
	}
}

HRESULT CKirbyBomb::Ready_HitBox()
{
	CCollider::COLLIDER_DESC desc{};
	desc.pOwner = this;
	desc.fHeight = m_fHitHeight * 1.1f;
	desc.fRadius = m_fHitRadius * 1.1f;
	desc.vCenter = m_vCenterOffset;
	desc.vRadians = m_vRadians;

	m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag, TEXT("Com_HitBox"), &desc);

	if (m_pHitBox == nullptr)
		return E_FAIL;

	m_pHitBox->Set_OnEnter(
		[this](CCollider* pOther)
		{
			if (!m_bAlive)
				return;

			CGameObject* pGameObject = pOther->Get_Owner();
			_uint iRenderGroup = pOther->Get_RegisteredGroup();

			if (iRenderGroup == ETOUI(COLLISION_LAYER::MONSTER_HURT))
			{
				if (auto* pDamageable = dynamic_cast<IDamageable*>(pGameObject))
				{
					ATTACK_INFO tAttackInfo{};
					tAttackInfo.eHitType = HIT_TYPE::BOMB;
					tAttackInfo.fDamage = m_fDamage;
					tAttackInfo.fKnockback = m_fKnockback;
					XMStoreFloat3(&tAttackInfo.vAttackerPos,
						m_pTransformCom->Get_State(STATE::POSITION));
					tAttackInfo.pAttacker = this;

					pDamageable->Damaged(tAttackInfo);
				}

				return;
			}
			else if (iRenderGroup == ETOUI(COLLISION_LAYER::PLAYER_BOMB))
			{
				int a = 10;
			}

			On_Impact();
		});

	m_pHitBox->Set_Enabled(false);
	m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_BOMB));



	return S_OK;
}

CKirbyBomb* CKirbyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKirbyBomb* pInstance = new CKirbyBomb(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CKirbyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKirbyBomb::Clone(void* pArg)
{
	CKirbyBomb* pInstance = new CKirbyBomb(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CKirbyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKirbyBomb::Free()
{
	__super::Free();
}
