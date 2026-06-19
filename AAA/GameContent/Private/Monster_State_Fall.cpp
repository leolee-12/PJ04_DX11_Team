#include "Monster_State_Fall.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Fall::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Fall::Get_StateType()
{
	return MONSTER_STATE_TYPE::FALL;
}

void CMonster_State_Fall::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
		pAnim->Play(&m_PlayInfo);
}

void CMonster_State_Fall::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (nullptr == pMonster)
		return;

	CMonster_Movement* pMove = pMonster->Get_Movement();

	// TODO : 바운스 도중 Fall - Landing 전환 고려하기

	if (pMove != nullptr && pMove->Is_Grounded())
		pMonster->Change_State(MONSTER_STATE_TYPE::LANDING);
}

void CMonster_State_Fall::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;
}

CMonster_State_Fall* CMonster_State_Fall::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Fall* pInstance = new CMonster_State_Fall();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Fall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Fall::Free()
{
	__super::Free();
}
