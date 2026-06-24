#include "Monster_State_Retreat.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Retreat::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	// Brain이 해당 애니메이션 동안에는 전이 못하게 막아야함
	m_bIsInterruptible = false;
	m_bUseMoveWindow = true;

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

	__super::Enter();

	if (m_pMovement)
	{
		m_pMovement->Set_LockFacing(true);		// 바라보던 방향 유지하며 뒷 걸음질
	}

	// 후퇴 방향 진입 할 때 1회 스냅샷 (facing 잠금이라 고정
	_vector vLook = m_pOwner->Get_Transform()->Get_State(STATE::LOOK);
	XMStoreFloat3(&m_MoveDir, XMVectorNegate(XMVectorSetY(vLook, 0.f)));
}

void CMonster_State_Retreat::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && m_pAnimator->Is_Overlay_Finished(0))
	{
		m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
		return;
	}

	__super::Update(fTimeDelta);
}

void CMonster_State_Retreat::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;

	__super::Exit(eNextState);

	if (m_pMovement)
		m_pMovement->Set_LockFacing(false);
}

void CMonster_State_Retreat::Apply_Movement(_float fTimeDelta)
{
	m_pOwner->Add_MoveDir(m_MoveDir);
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
