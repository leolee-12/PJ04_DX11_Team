#include "Monster_State_Landing.h"
#include "Monster.h"

HRESULT CMonster_State_Landing::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Landing::Get_StateType()
{
	return MONSTER_STATE_TYPE::LANDING;
}

void CMonster_State_Landing::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
		pAnim->Play(&m_PlayInfo);
}

void CMonster_State_Landing::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (pMonster == nullptr)
		return;

	CAnimator* pAnim = pMonster->Get_BodyAnimator();

	if (pAnim != nullptr && pAnim->Is_Finished())
		pMonster->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CMonster_State_Landing::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;
}

CMonster_State_Landing* CMonster_State_Landing::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Landing* pInstance = new CMonster_State_Landing();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Landing");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Landing::Free()
{
	__super::Free();
}
