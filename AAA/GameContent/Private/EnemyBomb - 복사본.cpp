#include "EnemyBomb.h"
#include "Animator.h"
#include "Model.h"
#include "Projectile_Movement.h"

CEnemyBomb::CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb { pDevice , pContext }
{
	m_fSpeed = 12.f;
	m_fLifeTime = 4.f;
	m_fDamage = 2.f;
	m_fKnockback = 6.f;
	m_fHitRadius = 0.5f;
}

CEnemyBomb::CEnemyBomb(const CEnemyBomb& Prototype)
	: CProjectile_Bomb ( Prototype ) 
{
}

HRESULT CEnemyBomb::Ready_Visual()
{
	if (FAILED(__super::Ready_Visual()))
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));

	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC ad{};
	ad.pModel = m_pModelCom;
	ad.strDataFile = TEXT("");
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad)))
		return E_FAIL;



	return S_OK;
}

void CEnemyBomb::On_Bounce(_int iCount)
{
	CAnimator::LAYER_PLAY_INFO Info{};
	Info.iSlot = 1;
	Info.tAnim.strAniName = "DangerGlow";
	Info.tAnim.bLoop = true;
	Info.tAnim.fSpeed = 1.25f;
	Info.Roots = { "BombM" };

	if (iCount == 1 && m_pAnimatorCom)
		m_pAnimatorCom->Apply_Overlay(Info);
}

void CEnemyBomb::Update_Terminal(_float dt)
{
	// 비행 끝난 후 진행  
	if (m_pAnimatorCom && m_pAnimatorCom->Is_Overlay_Finished(0))
		Kill();
}

void CEnemyBomb::Tick_Visual(_float fTimeDelta)
{
	if (m_bFlying && m_pAnimatorCom && m_pMovement)
	{
		_float3 vVel = m_pMovement->Get_Velocity();          // 감쇠 반영된 현재 속도
		_float  fHoriz = sqrtf(vVel.x * vVel.x + vVel.z * vVel.z);

		m_fRollAngle += fHoriz * ROLL_DEG_PER_SPEED * fTimeDelta;  // 속도에 비례
		if (m_fRollAngle >= 360.f)
			m_fRollAngle = fmodf(m_fRollAngle, 360.f);

		m_pAnimatorCom->SetBoneRotation("RotL", m_fRollAngle,
			XMVectorSet(1.f, 0.f, 0.f, 0.f));
	}

	if (m_pAnimatorCom)
		m_pAnimatorCom->Update(fTimeDelta);
}

void CEnemyBomb::On_Activated()
{
	if (m_pAnimatorCom)
	{
		m_pAnimatorCom->Clear_Overlay(1);
		m_pAnimatorCom->Play("FuseBurning", true);
	}
}

void CEnemyBomb::On_Impact()
{
	Kill();
}

CEnemyBomb* CEnemyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnemyBomb* pInstance = new CEnemyBomb(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnemyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnemyBomb::Clone(void* pArg)
{
	CEnemyBomb* pInstance = new CEnemyBomb(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnemyBomb");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnemyBomb::Free()
{
	__super::Free();
}
