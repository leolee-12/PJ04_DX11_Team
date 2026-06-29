#include "EnemyBomb.h"
#include "Animator.h"
#include "Model.h"
#include "Projectile_Movement.h"

CEnemyBomb::CEnemyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile_Bomb { pDevice , pContext }
{
	m_fSpeed = 25.f;
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

	m_pAnimatorCom->Play("FuseBurning", false, true);		// 크래쉬 안나게 설정

	// Overlay Animation 
	CAnimator::LAYER_PLAY_INFO LayerInfo{};
	LayerInfo.iSlot = 1;
	LayerInfo.tAnim.strAniName = "FuseBurning";
	LayerInfo.tAnim.bLoop = false;		// 해당 애니메이션 끝나면 수명 끝이므로 false
	LayerInfo.tAnim.bRestart = true;
	LayerInfo.tAnim.fSpeed = 1.25f;
	LayerInfo.Roots = { "EffectL" };

	m_pAnimatorCom->Apply_Overlay(LayerInfo);

	if (m_bCarried)
	{
		// 점화 이펙트 여기에서 부착

		m_pAnimatorCom->Pause_Mask(1);
		Update_Socket();
	}
}

void CEnemyBomb::On_Bounce(_int iCount)
{
	// 첫 착지만
	if (iCount != 1 || nullptr == m_pAnimatorCom)
		return;

	// Base Animation 
	CAnimator::ANI_PLAY_INFO AniInfo{};
	AniInfo.strAniName = "DangerGlow";
	AniInfo.bLoop = true;
	AniInfo.fSpeed = 1.25f;

	m_pAnimatorCom->Play(&AniInfo);			// Base 애니메이션 설정
	m_pAnimatorCom->Resume_Mask(1);
}

void CEnemyBomb::On_Explode()
{
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

	// TODO : 폭발 이벤트
	// TODO 폭발 시 추가 콜라이더 생성 (선택사항)
}

void CEnemyBomb::Be_Captured(CGameObject* pInhaler)
{
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


