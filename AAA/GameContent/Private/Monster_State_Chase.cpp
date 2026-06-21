#include "Monster_State_Chase.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Chase::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CMonster_State_Chase::Enter()
{
	if (m_pOwner == nullptr)
		return;

	if (m_pMovement)
		m_pMovement->Set_MoveSpeed(m_fSpeed);

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Chase::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();

	if (BB.pTarget == nullptr)
		return;

	// TODO : 지평면에서만 움직이는 애들 / 공중에서 추격하는 애들 구분할 것
	m_pOwner->Add_MoveDir(BB.vDirToTargetXZ);
}

void CMonster_State_Chase::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_Chase* CMonster_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Chase* pInstance = new CMonster_State_Chase();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Chase::Free()
{
	__super::Free();
}
