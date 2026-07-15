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
	m_eState = BOMB_STATE::NONE;	// 풀 재사용 리셋

	if (m_bCarried)
		Change_State(BOMB_STATE::HELD);
}

void CKirbyBomb::On_Launched()
{
	Change_State(BOMB_STATE::FLYING);
}

void CKirbyBomb::On_Bounce(_int iCount)
{
	if (iCount == 1)
		Change_State(BOMB_STATE::DANGER);
}

void CKirbyBomb::Change_State(BOMB_STATE eNext)
{
	if (m_eState == eNext)
		return;

	Exit_State(m_eState);

	m_eState = eNext;

	Enter_State(eNext);
}

void CKirbyBomb::Enter_State(BOMB_STATE eState)
{
	switch (eState)
	{
		case BOMB_STATE::HELD:
		{
			Reset_BombVisual();
			Play_BodyAnim(ANIM_FUSE, false);
			Start_Fuse();		// Overlay
			Pause_Fuse();		// Overlay Pause
			Update_Socket();
			Spawn_FuseFx();
			break;
		}
		case BOMB_STATE::FLYING:
		{
			Resume_Fuse(); // Overlay 다시 재생
			break;
		}
		case BOMB_STATE::DANGER:
		{
			Play_DangerGlow();
			break;
		}
	}
}

void CKirbyBomb::Update_State(_float fTimeDelta)
{
	//switch (m_eState)
	//{

	//}
}

void CKirbyBomb::Exit_State(BOMB_STATE eState)
{
	//switch (m_eState)
	//{
	
	//}
}

HRESULT CKirbyBomb::Ready_AnimEvents()
{
	//m_pAnimatorCom->Set_EventCallback(
	//	[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
	//	{
	//		switch (static_cast<EANIM_EVENT>(e.iEventType))
	//		{
	//		case EANIM_EVENT::SetBody:
	//			if (phase == ANIM_EVENT_PHASE::POINT)
	//				m_vGlow = _float3(e.vOffset.x, e.vOffset.y, e.vOffset.z);
	//			break;
	//		default:
	//			break;
	//		}
	//	}
	//);

	return S_OK;
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