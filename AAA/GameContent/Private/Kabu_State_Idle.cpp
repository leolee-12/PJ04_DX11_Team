#include "Kabu_State_Idle.h"
#include "Monster.h"
#include "Kabu.h"
#include "Monster_RailMovement.h"

HRESULT CKabu_State_Idle::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CKabu_State_Idle::Get_StateType()
{
	return MONSTER_STATE_TYPE::IDLE;
}

void CKabu_State_Idle::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CKabu_State_Idle::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr || nullptr == m_pMovement)
		return;

	if (!static_cast<CKabu*>(m_pOwner)->Is_Visible())
		return;

	static_cast<CMonster_RailMovement*>(m_pMovement)->Update_RailFollow(fTimeDelta);	
}

void CKabu_State_Idle::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CKabu_State_Idle* CKabu_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CKabu_State_Idle* pInstance = new CKabu_State_Idle();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CKabu_State_Idle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKabu_State_Idle::Free()
{
	__super::Free();
}
