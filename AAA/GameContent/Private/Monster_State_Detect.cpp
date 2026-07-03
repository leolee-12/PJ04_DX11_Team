#include "Monster_State_Detect.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Detect::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Detect::Get_StateType()
{
	return MONSTER_STATE_TYPE::DETECT;
}

void CMonster_State_Detect::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);

	m_pOwner->Play_OneShotSFX(L"CharaBasic_Find.wav", 0.6f);
}

void CMonster_State_Detect::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	// Å¸°Ù ¹æÇâ ½º³À¼¦ 
	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
	if (BB.pTarget != nullptr && m_pMovement)
		m_pMovement->Face_Smooth(XMLoadFloat3(&BB.vTargetPos), fTimeDelta);

	if (m_pAnimator && m_pAnimator->Is_Finished())
	{
		m_pOwner->Change_State(m_eNextState);
	}
}

void CMonster_State_Detect::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_Detect* CMonster_State_Detect::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Detect* pInstance = new CMonster_State_Detect();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Detect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Detect::Free()
{
	__super::Free();
}
