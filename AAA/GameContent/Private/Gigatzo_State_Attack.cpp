#include "Gigatzo_State_Attack.h"
#include "Monster.h"

HRESULT CGigatzo_State_Attack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_eNextState = MONSTER_STATE_TYPE::IDLE;
	return S_OK;
}

MONSTER_STATE_TYPE CGigatzo_State_Attack::Get_StateType()
{
	return MONSTER_STATE_TYPE::ATTACK;
}

void CGigatzo_State_Attack::Enter(MONSTER_STATE_TYPE ePrevState)
{
	UNREFERENCED_PARAMETER(ePrevState);

	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	ANI_PLAY_INFO Info{};
	Info.strAniName = "Attack";
	Info.bLoop = false;
	Info.fSpeed = 1.f;
	m_pAnimator->Play(&Info);
}

void CGigatzo_State_Attack::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	if (m_pAnimator->Is_Finished())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CGigatzo_State_Attack::Exit(MONSTER_STATE_TYPE eNextState)
{
	UNREFERENCED_PARAMETER(eNextState);
}

CGigatzo_State_Attack* CGigatzo_State_Attack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CGigatzo_State_Attack* pInstance = new CGigatzo_State_Attack();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CGigatzo_State_Attack");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGigatzo_State_Attack::Free()
{
	__super::Free();
}
