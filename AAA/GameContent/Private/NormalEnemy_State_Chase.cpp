#include "NormalEnemy_State_Chase.h"
#include "Monster.h"
#include "Transform.h"


HRESULT CNormalEnemy_State_Chase::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	// 자체 체인이라 중간에 못 끊게 
	m_bIsInterruptible = false;

	return S_OK;
}

MONSTER_STATE_TYPE CNormalEnemy_State_Chase::Get_StateType()
{
	return MONSTER_STATE_TYPE::CHASE;
}

void CNormalEnemy_State_Chase::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	__super::Enter(ePrevState);

	m_fTimer = 0.f;

	// 초기 돌격 방향 설정
	const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
	_vector vDir = XMVectorSetY(XMLoadFloat3(&BB.vDirToTargetXZ), 0.f);

	if (XMVector3LessOrEqual(XMVector3LengthSq(vDir), XMVectorReplicate(1e-6f)))
		vDir = XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f);

	XMStoreFloat3(&m_MoveDir, XMVector3Normalize(vDir));
}

void CNormalEnemy_State_Chase::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer += fTimeDelta;

	// 총 시간 초과 -> FIND(BRAKE는 이 분기 앞에 삽입)
	if (m_fTimer >= s_fChaseTime)
	{
		m_pOwner->Change_State(MONSTER_STATE_TYPE::BRAKE);
		return;
	}

	// 추적 Phase
	if (m_fTimer < s_fTrackTime)
	{
		const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
		_vector vDir = XMLoadFloat3(&BB.vDirToTargetXZ);

		if (BB.pTarget != nullptr && XMVector3Greater(XMVector3LengthSq(vDir), XMVectorReplicate(1e-6f)))
			m_MoveDir = BB.vDirToTargetXZ;
	}

	// 이동 
	__super::Update(fTimeDelta);
}

void CNormalEnemy_State_Chase::Apply_Movement(_float fTimeDelta)
{
	m_pOwner->Add_MoveDir(XMLoadFloat3(&m_MoveDir));
}

CNormalEnemy_State_Chase* CNormalEnemy_State_Chase::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CNormalEnemy_State_Chase* pInstance = new CNormalEnemy_State_Chase();

	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CNormalEnemy_State_Chase");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNormalEnemy_State_Chase::Free()
{
	__super::Free();
}
