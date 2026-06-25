#include "Monster_State_Attack.h"
#include "Monster_Movement.h"
#include "Monster.h"
#include "Transform.h"

HRESULT CMonster_State_Attack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

void CMonster_State_Attack::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	// Å¸°Ù ¹æÇâ ½º³À¼¦ 
	m_MoveDir = m_pOwner->Get_BlackBoard().vDirToTargetXZ;

	_vector vDir = XMLoadFloat3(&m_MoveDir);
	if (!XMVector3Equal(vDir, XMVectorZero()))
	{
		_vector vMyPos = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
		m_pOwner->Get_Transform()->LookAt(vMyPos + vDir);
	}

	if (m_pMovement)
		m_pMovement->Set_MoveSpeed(m_fSpeed);

	if (m_pAnimator)
		Play_AttackAnimation();
}

void CMonster_State_Attack::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	if (m_pAnimator && m_pAnimator->Is_Finished())
	{
		m_pOwner->Change_State(m_eNextState);
		return;
	}

	if (m_pOwner->Get_BlackBoard().bCanMove)
		m_pOwner->Add_MoveDir(m_MoveDir);
}

void CMonster_State_Attack::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

void CMonster_State_Attack::Free()
{
	__super::Free();
}
