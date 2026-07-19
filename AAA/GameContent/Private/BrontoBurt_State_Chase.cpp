#include "BrontoBurt_State_Chase.h"
#include "BrontoBurt.h"
#include "Monster_Movement.h"

HRESULT CBrontoBurt_State_Chase::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CBrontoBurt_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CBrontoBurt_State_Chase::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CBrontoBurt_State_Chase::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner || nullptr == m_pMovement)
		return;

	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
	if (nullptr == BB.pTarget)
		return;                                   
	_vector vPlayer = XMVectorSetW(XMLoadFloat3(&BB.vTargetPos), 1.f);

	m_pMovement->Face_Smooth(vPlayer, fTimeDelta);
	m_pMovement->Fly_Toward(vPlayer, s_fChaseSpeed, fTimeDelta, 0.5f);
}

void CBrontoBurt_State_Chase::Exit(MONSTER_STATE_TYPE eNextState)
{
}

CBrontoBurt_State_Chase* CBrontoBurt_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBrontoBurt_State_Chase* pInstance = new CBrontoBurt_State_Chase();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBrontoBurt_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBrontoBurt_State_Chase::Free()
{
	__super::Free();
}
