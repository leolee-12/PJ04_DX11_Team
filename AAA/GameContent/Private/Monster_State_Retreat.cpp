#include "Monster_State_Retreat.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Retreat::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Retreat::Get_StateType()
{
	return MONSTER_STATE_TYPE::RETREAT;
}

void CMonster_State_Retreat::Enter()
{
	if (m_pOwner == nullptr)
		return;


	if (m_pMovement)
	{
		m_pMovement->Set_MoveSpeed(m_fSpeed);
		m_pMovement->Set_LockFacing(true);		// 바라보던 방향 유지하며 뒷 걸음질
	}

	if (m_pAnimator)
		m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Retreat::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);

	if (!m_pOwner->Get_BlackBoard().bMoveLocked)
	{
		_vector vLook = m_pOwner->Get_Transform()->Get_State(STATE::LOOK);

		_float3 vBack{};
		XMStoreFloat3(&vBack, XMVectorNegate(XMVectorSetY(vLook, 0.f)));
		m_pOwner->Add_MoveDir(vBack);
	}
}

void CMonster_State_Retreat::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pMovement)
		m_pMovement->Set_LockFacing(false);
}

CMonster_State_Retreat* CMonster_State_Retreat::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Retreat* pInstance = new CMonster_State_Retreat();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Retreat");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Retreat::Free()
{
	__super::Free();
}
