#include "Monster_State_Patrol.h"
#include "Monster.h"
#include "Transform.h"

MONSTER_STATE_TYPE CMonster_State_Patrol::Get_StateType()
{
	return MONSTER_STATE_TYPE::PATROL;
}

void CMonster_State_Patrol::Enter(MONSTER_STATE_TYPE ePrevState)
{
	__super::Enter(ePrevState);

	m_fTimer = 0.f;
	Pick_WanderTarget();
}

void CMonster_State_Patrol::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer += fTimeDelta;

	_vector vToTarget	= Offset_To_XZ(m_vWanderTarget);
	_vector vDistSq		= XMVector3LengthSq(vToTarget);
	_vector vArriveSq	= XMVectorReplicate(s_fArriveDist * s_fArriveDist);

	_bool bArrived		= XMVector3LessOrEqual(vDistSq, vArriveSq);
	_bool bTimeOut		= (m_fTimer >= s_fMaxWanderTime);

	// µµÂø or Å¸ÀÓ¾Æ¿ô -> IDLE º¹±Í
	if (bArrived || bTimeOut)
	{
		m_pOwner->Change_State(m_eNextState);
		return;
	}

	__super::Update(fTimeDelta);
}

void CMonster_State_Patrol::Apply_Movement(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	_vector vDir = Offset_To_XZ(m_vWanderTarget);

	m_pOwner->Add_MoveDir(XMVector3Normalize(vDir));
}

CMonster_State_Patrol* CMonster_State_Patrol::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Patrol* pInstance = new CMonster_State_Patrol();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Patrol");
		Safe_Release(pInstance);
	}


	return pInstance;
}

void CMonster_State_Patrol::Pick_WanderTarget()
{
	if (m_pOwner == nullptr)
		return;

	_vector vBase = XMLoadFloat3(&m_pOwner->Get_BasePos());
	_vector vMyPos = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);

	_vector vToBase = XMVectorSetY(XMVectorSubtract(vBase, vMyPos), 0.f);
	_vector vDistSq = XMVector3LengthSq(vToBase);
	_vector vRangeSq = XMVectorReplicate(s_fPatrolRange * s_fPatrolRange);

	_vector vTarget;

	if (XMVector3Greater(vDistSq, vRangeSq))
	{
		vTarget = vBase;		// Range ¹Û -> BasePos·Î º¹±Í
	}
	else
	{
		// BasePos Áß½É Range ³» ÀÓÀÇ ÁöÁ¡
		_float fAngle = (rand() % 1000) * 0.001f * XM_2PI;
		_float fRadius = (rand() % 1000) * 0.001f * s_fPatrolRange;

		_float fSin, fCos;
		XMScalarSinCos(&fSin, &fCos, fAngle);

		_vector vOffset =
			XMVectorSet(fCos * fRadius, 0.f, fSin * fRadius, 0.f);
		vTarget = XMVectorAdd(vBase, vOffset);
	}

	vTarget = XMVectorSetY(vTarget, XMVectorGetY(vMyPos));
	XMStoreFloat3(&m_vWanderTarget, vTarget);
}

_vector CMonster_State_Patrol::Offset_To_XZ(const _float3& vTargetF3) const
{
	_vector vMyPos = m_pOwner->Get_Transform()->Get_State(STATE::POSITION);
	_vector vTarget = XMLoadFloat3(&vTargetF3);
	return XMVectorSetY(XMVectorSubtract(vTarget, vMyPos), 0.f);
}

void CMonster_State_Patrol::Free()
{
	__super::Free();
}
