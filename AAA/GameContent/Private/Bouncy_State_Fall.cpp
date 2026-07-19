#include "Bouncy_State_Fall.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CBouncy_State_Fall::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CBouncy_State_Fall::Get_StateType()
{
	return MONSTER_STATE_TYPE::FALL;
}

void CBouncy_State_Fall::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	ANI_PLAY_INFO Info{};
	Info.strAniName = (ePrevState == MONSTER_STATE_TYPE::KNOCK_BACK) ? "Fall2" : "Fall";
	Info.bLoop = true;
	Info.fBlend = s_fFallBlend;
	Info.fSpeed = 1.f;
	m_pAnimator->Play(&Info);
}

void CBouncy_State_Fall::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	if (m_pMovement && m_pMovement->Is_Grounded())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::LANDING);
}

void CBouncy_State_Fall::Exit(MONSTER_STATE_TYPE eNextState)
{
}

CBouncy_State_Fall* CBouncy_State_Fall::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBouncy_State_Fall* pInstance = new CBouncy_State_Fall();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBouncy_State_Fall");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBouncy_State_Fall::Free()
{
	__super::Free();
}
