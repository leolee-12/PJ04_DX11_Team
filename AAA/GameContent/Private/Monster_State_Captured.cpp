#include "Monster_State_Captured.h"
#include "Monster.h"

CMonster_State_Captured::CMonster_State_Captured()
{
}

HRESULT	CMonster_State_Captured::Initialize()
{
	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Captured::Get_StateType()
{
	return MONSTER_STATE_TYPE::CAPTURED;
}

void CMonster_State_Captured::Enter(CMonster* pMonster)
{
	if (!pMonster) return;
	pMonster->Get_BlackBoard().bCanTransition = false;
	pMonster->Enable_Controller(false);
	pMonster->Enable_Colliders(false);
	pMonster->Play_StateAnimation(MONSTER_STATE_TYPE::CAPTURED);

	m_fPullSpeed = s_fPullInitSpeed;                        
	m_vBaseScale = pMonster->Get_Transform()->Get_Scaled(); 
	m_fScaleRatio = 1.f;
}

void CMonster_State_Captured::Update(CMonster* pMonster, _float fTimeDelta)
{
	if (!pMonster) return;
	CGameObject* pCaptor = pMonster->Get_Captor();
	if (!pCaptor) return;

	CTransform* pCapT = pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	CTransform* pT = pMonster->Get_Transform();
	_vector vSelf = pT->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float  fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f) { pMonster->On_Swallowed(); return; }

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	pT->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vDir) * fMove);

	m_fScaleRatio += (s_fMinScaleRatio - m_fScaleRatio) * min(s_fShrinkLerp * fTimeDelta, 1.f);
	pT->Set_Scale(m_vBaseScale.x * m_fScaleRatio,
		m_vBaseScale.y * m_fScaleRatio,
		m_vBaseScale.z * m_fScaleRatio);
}

void CMonster_State_Captured::Exit(CMonster* pMonster)
{
	if (!pMonster) return;
	pMonster->Get_Transform()->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);
	pMonster->Enable_Controller(true);
	pMonster->Enable_Colliders(true);
}

CMonster_State_Captured* CMonster_State_Captured::Create()
{
	CMonster_State_Captured* pInstance = new  CMonster_State_Captured();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created: CMonster_State_Captured");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Captured::Free()
{
	__super::Free();
}
