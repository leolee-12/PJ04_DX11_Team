#include "KirbyBomb.h"

#include "GameContent_Const.h"

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
	m_fHitRadius = 0.60f;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CKirbyBomb::Update(_float fTimeDelta)
{
	if (!m_bAlive)
		return;

	Update_State(fTimeDelta);
	__super::Update(fTimeDelta);
}

void CKirbyBomb::Tick_Visual(_float fTimeDelta)
{
	Roll_ByMovement(fTimeDelta);
	m_pAnimatorCom->Update(fTimeDelta);
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
			if(m_pAnimatorCom->Is_Finished())
			{
				if(m_iExplodeAniPlayCount <= 0)
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
	desc.fHeight = m_fHitHeight;
	desc.fRadius = m_fHitRadius;
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

			if (ETOUI(COLLISION_LAYER::MONSTER_HURT) != pOther->Get_RegisteredGroup())
				return;

			if (auto* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner()))
			{
				ATTACK_INFO tAttackInfo{};
				tAttackInfo.eHitType = HIT_TYPE::BOMB;
				tAttackInfo.fDamage = m_fDamage;
				tAttackInfo.fKnockback = m_fKnockback;
				XMStoreFloat3(&tAttackInfo.vAttackerPos,
					m_pTransformCom->Get_State(STATE::POSITION));
				tAttackInfo.pAttacker = this;

				pVictim->Damaged(tAttackInfo);
			}

			On_Impact();
		});

	m_pHitBox->Set_Enabled(false);
	m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE));



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