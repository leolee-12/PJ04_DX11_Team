#include "NormalEnemy_State_Brake.h"
#include "Monster.h"
#include "Monster_Movement.h"
#include "Transform.h"

HRESULT CNormalEnemy_State_Brake::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	// 감속 중에는 다른 행동으로 끊기지 않음
	m_bIsInterruptible = false;

	return S_OK;
}

MONSTER_STATE_TYPE CNormalEnemy_State_Brake::Get_StateType()
{
	return MONSTER_STATE_TYPE::BRAKE;
}

void CNormalEnemy_State_Brake::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner)
		return;

	__super::Enter(ePrevState);

	m_fTimer = 0.f;

	// 돌진하던 방향(현재 바라보는 방향
	_vector vDir = XMVectorSetY(m_pOwner->Get_Transform()->Get_State(STATE::LOOK), 0.f);

	if (XMVector3LessOrEqual(XMVector3LengthSq(vDir), XMVectorReplicate(1e-6f)))
		vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	XMStoreFloat3(&m_MoveDir, XMVector3Normalize(vDir));
}

void CNormalEnemy_State_Brake::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer += fTimeDelta;

	// 감속 종료 -> FIND
	if (m_fTimer >= s_fBrakeTime)
	{
		m_pOwner->Change_State(MONSTER_STATE_TYPE::FIND);
		return;
	}

	// 목표 이동속도를  m_fSpeed -> 0 으로 선형 감속
	if (m_pMovement)
	{
		const _float fRatio = 1.f - (m_fTimer / s_fBrakeTime); 
		m_pMovement->Set_MoveSpeed(m_fSpeed * fRatio);
	}

	__super::Update(fTimeDelta);
}

void CNormalEnemy_State_Brake::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

void CNormalEnemy_State_Brake::Apply_Movement(_float fTimeDelta)
{
	m_pOwner->Add_MoveDir(XMLoadFloat3(&m_MoveDir));
}

CNormalEnemy_State_Brake* CNormalEnemy_State_Brake::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CNormalEnemy_State_Brake* pInstance = new CNormalEnemy_State_Brake();

	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CNormalEnemy_State_Brake");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNormalEnemy_State_Brake::Free()
{
	__super::Free();
}
