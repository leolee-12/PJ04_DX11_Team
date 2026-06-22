#include "Monster_State_Idle.h"
#include "Monster.h"

HRESULT CMonster_State_Idle::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CMonster_State_Idle::Enter()
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Idle::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;
}

void CMonster_State_Idle::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_Idle* CMonster_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Idle* pInstance = new CMonster_State_Idle();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Idle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Idle::Free()
{
	__super::Free();
}
