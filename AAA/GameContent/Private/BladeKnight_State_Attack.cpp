#include "BladeKnight_State_Attack.h"
#include "BladeKnight.h"
#include "BladeKnight_Body.h"
#include "Monster_Movement.h"
#include "BladeKnight_Sword.h"

MONSTER_STATE_TYPE CBladeKnight_State_Attack::Get_StateType()
{
	return MONSTER_STATE_TYPE::ATTACK;
}

MONSTER_STATE_TYPE CBladeKnight_State_Attack::Get_NextState()
{
	return MONSTER_STATE_TYPE::RETREAT;
}

void CBladeKnight_State_Attack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;

	if (CBladeKnight* pBK = dynamic_cast<CBladeKnight*>(m_pOwner))
		if (pBK->Get_Sword())
			pBK->Get_Sword()->Set_HitBox(false);
}

void CBladeKnight_State_Attack::Play_AttackAnimation()
{
	if (m_pAnimator)
	{
		CAnimator::ANI_PLAY_INFO AniInfo{};
		AniInfo.strAniName = "AttackStart";
		AniInfo.bLoop = false;
		AniInfo.fSpeed = 1.25f;
		m_pAnimator->Play(&AniInfo); // Queue 클리어 + 시작

		AniInfo.strAniName = "Attack";
		AniInfo.fSpeed = 1.25f;
		m_pAnimator->Enqueue(AniInfo);
	}
}

CBladeKnight_State_Attack* CBladeKnight_State_Attack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBladeKnight_State_Attack* pInstance = new CBladeKnight_State_Attack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBladeKnight_State_Attack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight_State_Attack::Free()
{
	__super::Free();
}
