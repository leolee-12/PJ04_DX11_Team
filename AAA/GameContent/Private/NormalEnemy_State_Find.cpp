#include "NormalEnemy_State_Find.h"
#include "Monster.h"

HRESULT CNormalEnemy_State_Find::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = false;
	m_eNextState = MONSTER_STATE_TYPE::IDLE;

	return S_OK;
}

MONSTER_STATE_TYPE CNormalEnemy_State_Find::Get_StateType()
{
	return MONSTER_STATE_TYPE::FIND;
}

void CNormalEnemy_State_Find::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	// 진입 직전 상태로 애니메이션 분기
	ANI_PLAY_INFO Info{};
	Info.strAniName = (ePrevState == MONSTER_STATE_TYPE::BRAKE) ? "LookAroundAfterBrake" : "LookAround";
	Info.bLoop = false;
	Info.fSpeed = 1.5f;
	m_pAnimator->Play(&Info);
}

void CNormalEnemy_State_Find::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
		m_pOwner->Change_State(m_eNextState);
}

void CNormalEnemy_State_Find::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

CNormalEnemy_State_Find* CNormalEnemy_State_Find::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CNormalEnemy_State_Find* pInstance = new CNormalEnemy_State_Find();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CNormalEnemy_State_Find");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNormalEnemy_State_Find::Free()
{
	__super::Free();
}
