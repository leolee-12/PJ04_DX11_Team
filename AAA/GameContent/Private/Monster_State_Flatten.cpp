#include "Monster_State_Flatten.h"
#include "Monster.h"
#include "Transform.h"

HRESULT CMonster_State_Flatten::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_bIsInterruptible = false;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Flatten::Get_StateType()
{
	return MONSTER_STATE_TYPE::FLATTEN;
}

void CMonster_State_Flatten::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer = 0.f;
	m_vBaseScale = m_pOwner->Get_Transform()->Get_Scaled();

	// ±âÁ¸ ¾Ö´Ï¸ÞÀÌ¼Ç ¸ØÃã
	if (m_pAnimator)
	{
		m_pAnimator->Pause();
	}

	m_pOwner->Enable_Controller(false);   
	m_pOwner->Enable_Colliders(false);
}

void CMonster_State_Flatten::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	m_fTimer += fTimeDelta;

	_float t = m_fTimer / s_fSquishDur;
	if (t > 1.f) t = 1.f;

	_float fY = m_vBaseScale.y * (1.f - (1.f - s_fFlatY) * t);
	_float fXZ = 1.f + (s_fSpreadXZ - 1.f) * t;

	m_pOwner->Get_Transform()->Set_Scale(
		m_vBaseScale.x * fXZ, fY, m_vBaseScale.z * fXZ);

	if (m_fTimer >= s_fLifeTime)
		m_pOwner->Despawn();
}

void CMonster_State_Flatten::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;
}

CMonster_State_Flatten* CMonster_State_Flatten::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Flatten* pInstance = new CMonster_State_Flatten();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Flatten");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Flatten::Free()
{
	__super::Free();
}
