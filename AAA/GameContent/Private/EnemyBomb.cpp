#include "EnemyBomb.h"
#include "Animator.h"
#include "Model.h"
#include "Projectile_Movement.h"

CEnemyBomb::CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb { pDevice , pContext }
{
	m_fSpeed = 18.f;
	m_fLifeTime = 4.f;
	m_fDamage = 2.f;
	m_fKnockback = 4.f;
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

void CEnemyBomb::On_Activated()
{
	if (nullptr == m_pAnimatorCom)
		return;

	m_fRollAngle = 0.f;
	m_pAnimatorCom->Clear_Overlay(1);
	m_pAnimatorCom->Play("FuseBurning", true);

	if (m_bCarried)
		Update_Socket();
}

void CEnemyBomb::On_Bounce(_int iCount)
{
	// Ã¹ ÂøÁö¸¸
	if (iCount != 1 || nullptr == m_pAnimatorCom)
		return;

	CAnimator::ANI_PLAY_INFO AniInfo{};
	AniInfo.strAniName = "DangerGlow";
	AniInfo.fSpeed = 1.25f;
	AniInfo.bLoop = true;

	m_pAnimatorCom->Play(&AniInfo);

	CAnimator::LAYER_PLAY_INFO Info{};
	Info.iSlot = 1;
	Info.tAnim.strAniName = "FuseBurning";
	Info.tAnim.bLoop = false;
	Info.tAnim.fSpeed = 1.f;
	Info.Roots = { "EffectL", "FuseM"};

	m_pAnimatorCom->Apply_Overlay(Info);
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
