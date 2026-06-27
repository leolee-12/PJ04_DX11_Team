#include "PoppyBrosJr_State_Idle.h"
#include "Monster_Movement.h"
#include "PoppyBrosJr.h"

void CPoppyBrosJr_State_Idle::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	const _int iAIType = static_cast<CPoppyBrosJr*>(m_pOwner)->Get_AIType();
	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
	const _float fAbsHeight = fabsf(BB.fHeightToTarget);
	const _bool bLookable =  (BB.fDistToTargetXZ <= 7.f && fAbsHeight < 3.f);

	if (iAIType != 0)
	{
		const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
		if (nullptr != BB.pTarget && BB.fDistToTargetXZ > m_fFaceDeadZoneXZ && bLookable)
		{
			m_pMovement->Face_Smooth(XMLoadFloat3(&BB.vTargetPos), fTimeDelta);
		}
	}
}

CPoppyBrosJr_State_Idle* CPoppyBrosJr_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CPoppyBrosJr_State_Idle* pInstance = new CPoppyBrosJr_State_Idle();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CPoppyBrosJr_State_Idle");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPoppyBrosJr_State_Idle::Free()
{
	__super::Free();
}
