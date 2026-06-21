#include "Monster_State_Fall.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Fall::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Fall::Get_StateType()
{
	return MONSTER_STATE_TYPE::FALL;
}

void CMonster_State_Fall::Enter()
{
	if (m_pOwner == nullptr)
		return;
	
	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Fall::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	if (m_pMovement && m_pMovement->Is_Grounded())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::LANDING);
}

void CMonster_State_Fall::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_Fall* CMonster_State_Fall::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Fall* pInstance = new CMonster_State_Fall();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Fall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Fall::Free()
{
	__super::Free();
}
