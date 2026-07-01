#include "PoppyBrosJr_State_WindUp.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CPoppyBrosJr_State_WindUp::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_fMaxTime = 4.f;
	m_eNextState = MONSTER_STATE_TYPE::ATTACK;

	return S_OK;
}

MONSTER_STATE_TYPE CPoppyBrosJr_State_WindUp::Get_StateType()
{
	return MONSTER_STATE_TYPE::WINDUP;
}

void CPoppyBrosJr_State_WindUp::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	m_fMaxTime = (ePrevState == MONSTER_STATE_TYPE::ATTACK) ? 4.f : 2.f;

	ANI_PLAY_INFO Info{};
	Info.strAniName = "EnemyWait2";
	Info.bLoop = true;
	Info.fSpeed = 1.5f;	

	m_pAnimator->Play(&Info);
}

void CPoppyBrosJr_State_WindUp::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();

	if (nullptr == BB.pTarget)
	{
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
		return;
	}

	m_fMaxTime -= fTimeDelta;

	if (m_fMaxTime <= 0.f)
	{
		m_pOwner->Change_State(m_eNextState);
		return;
	}


	if (m_pOwner->Get_AIType() != 0 && m_pMovement)
		m_pMovement->Face_Smooth(XMLoadFloat3(&BB.vTargetPos), fTimeDelta);
}

void CPoppyBrosJr_State_WindUp::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

CPoppyBrosJr_State_WindUp* CPoppyBrosJr_State_WindUp::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CPoppyBrosJr_State_WindUp* pInstance = new CPoppyBrosJr_State_WindUp();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CPoppyBrosJr_State_WindUp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPoppyBrosJr_State_WindUp::Free()
{
	__super::Free();
}
