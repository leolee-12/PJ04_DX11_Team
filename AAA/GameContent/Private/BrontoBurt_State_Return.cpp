#include "BrontoBurt_State_Return.h"
#include "BrontoBurt.h"
#include "Monster_RailMovement.h"
#include "Monster_Movement.h"

HRESULT CBrontoBurt_State_Return::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = true;

	return S_OK;
}

MONSTER_STATE_TYPE CBrontoBurt_State_Return::Get_StateType()
{
	return MONSTER_STATE_TYPE::RETURN;
}

void CBrontoBurt_State_Return::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner)
		return;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CBrontoBurt_State_Return::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner || nullptr == m_pMovement)
		return;

	// AI Type : 1 - 플레이어 추격형 
	if (m_pOwner->Get_AIType() == 1)
	{
		_float3 vBaseF = m_pOwner->Get_BasePos();
		_vector vBase = XMVectorSetW(XMLoadFloat3(&vBaseF), 1.f);

		m_pMovement->Face_Smooth(vBase, fTimeDelta);
		if (m_pMovement->Fly_Toward(vBase, s_fReturnSpeed, fTimeDelta, 0.3f))
			m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
		return;
	}

	// AI Type : 0 - 레일 이동형

	auto pRail = static_cast<CMonster_RailMovement*>(m_pMovement);
	if (pRail->Return_TowardPath(fTimeDelta, s_fReturnSpeed))
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CBrontoBurt_State_Return::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

CBrontoBurt_State_Return* CBrontoBurt_State_Return::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CBrontoBurt_State_Return* pInstance = new CBrontoBurt_State_Return();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CBrontoBurt_State_Return");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBrontoBurt_State_Return::Free()
{
	__super::Free();
}
