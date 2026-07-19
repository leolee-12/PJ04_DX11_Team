#include "BrontoBurt_Brain.h"
#include "BrontoBurt.h"
#include "Monster_RailMovement.h"
#include "Transform.h"

CBrontoBurt_Brain::CBrontoBurt_Brain()
{
}

HRESULT CBrontoBurt_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CBrontoBurt_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	if (!Can_Decide(BlackBoard))
		return;

	CMonster_Movement* pMove = static_cast<CMonster_Movement*>(m_pOwner->Get_Movement());
	if (pMove && pMove->Is_Launched())
	{
		m_fTimer += fTimeDelta;
		if (m_fTimer >= s_fFallRecoverTime)
		{
			m_fTimer = 0.f;
			pMove->Cancle_Launch();
			if (m_pOwner->Has_State(MONSTER_STATE_TYPE::RETURN))
				m_pOwner->Change_State(MONSTER_STATE_TYPE::RETURN);
		}
		return;
	}
	m_fTimer = 0.f;

	if (m_pOwner->Get_AIType() == 1)
	{
		const MONSTER_STATE_TYPE eCur = m_pOwner->Get_StateType();
		const _bool bTarget = (nullptr != BlackBoard.pTarget);

		_float3 vBaseF = m_pOwner->Get_BasePos();
		_vector vPos = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
		_vector vXZ = XMVectorSetY(XMVectorSubtract(vPos, XMLoadFloat3(&vBaseF)), 0.f);
		_float  fXZ = XMVectorGetX(XMVector3Length(vXZ));
		_float  fDy = fabsf(XMVectorGetY(vPos) - vBaseF.y);
		const _bool bInLeash = (fXZ <= 40.f) && (fDy <= 20.f);

		if (bTarget && bInLeash)
		{
			if (eCur == MONSTER_STATE_TYPE::IDLE)
				m_pOwner->Change_State(MONSTER_STATE_TYPE::DETECT);   
		}
		else
		{
			if (eCur == MONSTER_STATE_TYPE::CHASE)
				m_pOwner->Change_State(MONSTER_STATE_TYPE::RETURN);  
		}
		return;
	}

	// AIType 0: current rail return logic.
	if (m_pOwner->Get_StateType() != MONSTER_STATE_TYPE::IDLE)
		return;

	if (!m_pOwner->Has_State(MONSTER_STATE_TYPE::RETURN))
		return;

	auto pRail = static_cast<CMonster_RailMovement*>(m_pOwner->Get_Movement());
	if (nullptr == pRail || !pRail->Has_Rail())
		return;

	if (pRail->Is_OffPath())
		m_pOwner->Change_State(MONSTER_STATE_TYPE::RETURN);
}

CBrontoBurt_Brain* CBrontoBurt_Brain::Create(CMonster* pOwner)
{
    CBrontoBurt_Brain* pInstance = new CBrontoBurt_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CBrontoBurt_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBrontoBurt_Brain::Free()
{
    __super::Free();
}
