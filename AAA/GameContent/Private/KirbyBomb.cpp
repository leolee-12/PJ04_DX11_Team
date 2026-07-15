#include "KirbyBomb.h"

#include "GameContent_const.h"
#include "Projectile_Movement.h"

#include "Effect_Loader.h"

CKirbyBomb::CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb{ pDevice , pContext }
{
}

CKirbyBomb::CKirbyBomb(const CKirbyBomb& Prototype)
	: CProjectile_Bomb(Prototype)
{
}

HRESULT CKirbyBomb::Initialize_Prototype()
{
	return S_OK;;
}

HRESULT CKirbyBomb::Initialize(void* pArg)
{
	// Launch 檬扁 加档
	//m_fSpeed = 25.f;
	//m_fDamage = 2.f;
	//m_fKnockback = 4.f;
	//m_fHitRadius = 0.60f;

	// Transform, Movement, HitBox, Shader 积己 
	// Model Animator 积己
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;	

	m_pAnimatorCom->Play("FuseBurning", false, true);

	return S_OK;;
}

void CKirbyBomb::Update(_float fTimeDelta)
{
	if (!m_bAlive)
		return;

	__super::Update(fTimeDelta);

	Update_FuseSocket();
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
	//tDesc.strDataFile = TEXT("");
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (m_pAnimatorCom == nullptr || FAILED(m_pAnimatorCom->Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CKirbyBomb::On_Activated()
{
}

void CKirbyBomb::On_Bounce(_int iCount)
{
}

void CKirbyBomb::On_Explode()
{

}

void CKirbyBomb::Update_FuseSocket()
{

}

void CKirbyBomb::Despawn()
{

}

HRESULT CKirbyBomb::Ready_AnimEvents()
{
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

	m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
		TEXT("Com_HitBox"), &desc);
	if (m_pHitBox == nullptr)
		return E_FAIL;

	m_pHitBox->Set_OnEnter([this](CCollider* pOther)
		{
			if (!m_bAlive)
				return;

			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
				return;

			if (auto* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner()))
			{
				ATTACK_INFO tAttackInfo{};
				tAttackInfo.eHitType = HIT_TYPE::BOMB;
				tAttackInfo.fDamage = m_fDamage;
				tAttackInfo.fKnockback = m_fKnockback;
				XMStoreFloat3(&tAttackInfo.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
				tAttackInfo.pAttacker = this;
				pVictim->Damaged(tAttackInfo);
			}

			On_Impact();
		}
	);

	m_pHitBox->Set_Enabled(false);
	m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_HIT));

	return S_OK;
}

CKirbyBomb* CKirbyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKirbyBomb* pInstance = new CKirbyBomb(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKirbyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKirbyBomb::Clone(void* pArg)
{
	CKirbyBomb* pInstance = new CKirbyBomb(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CKirbyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKirbyBomb::Free()
{
	__super::Free();
}