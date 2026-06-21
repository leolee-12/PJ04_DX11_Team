#include "Monster_State_Attack.h"
#include "Monster.h"

HRESULT CMonster_State_Attack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Attack::Get_StateType()
{
	return MONSTER_STATE_TYPE::ATTACK;
}

void CMonster_State_Attack::Enter()
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Attack::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;


	if (m_pAnimator && m_pAnimator->Is_Finished())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CMonster_State_Attack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_Attack* CMonster_State_Attack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Attack* pInstance = new CMonster_State_Attack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Attack");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Attack::Free()
{
	__super::Free();
}
