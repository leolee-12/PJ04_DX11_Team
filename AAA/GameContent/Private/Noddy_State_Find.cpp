#include "Noddy_State_Find.h"
#include "Monster.h"

HRESULT CNoddy_State_Find::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = false;
	return S_OK;
}

MONSTER_STATE_TYPE CNoddy_State_Find::Get_StateType()
{
	return MONSTER_STATE_TYPE::FIND;
}

void CNoddy_State_Find::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CNoddy_State_Find::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::SLEEP_START);
}

void CNoddy_State_Find::Exit(MONSTER_STATE_TYPE eNextState)
{
}

CNoddy_State_Find* CNoddy_State_Find::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CNoddy_State_Find* pInstance = new CNoddy_State_Find();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CNoddy_State_Find");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CNoddy_State_Find::Free()
{
	__super::Free();
}
