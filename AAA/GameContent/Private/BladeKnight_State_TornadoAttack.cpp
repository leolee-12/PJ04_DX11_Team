#include "BladeKnight_State_TornadoAttack.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Animator.h"
#include "Monster_Movement.h"
#include "BladeKnight_Sword.h"

MONSTER_STATE_TYPE CBladeKnight_State_TornadoAttack::Get_StateType()
{
	return MONSTER_STATE_TYPE::TORNADO_ATTACK;
}

void CBladeKnight_State_TornadoAttack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;

	if (CBladeKnight* pBK = dynamic_cast<CBladeKnight*>(m_pOwner))
		if (pBK->Get_Sword())
			pBK->Get_Sword()->Set_HitBox(false);
}

void CBladeKnight_State_TornadoAttack::Play_AttackAnimation()
{
	if (m_pAnimator)
	{
		CAnimator::ANI_PLAY_INFO AniInfo{};
		AniInfo.strAniName = "TornadoAttackCharge";
		AniInfo.bLoop = false;
		AniInfo.fSpeed = 1.50f;
		m_pAnimator->Play(&AniInfo); // Queue 클리어 + 시작

		AniInfo.strAniName = "TornadoAttack";
		AniInfo.fSpeed = 1.0f;
		m_pAnimator->Enqueue(AniInfo);
	}
}

CBladeKnight_State_TornadoAttack* CBladeKnight_State_TornadoAttack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBladeKnight_State_TornadoAttack* pInstance = new CBladeKnight_State_TornadoAttack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_TornadoAttack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_TornadoAttack::Free()
{
	__super::Free();
}
