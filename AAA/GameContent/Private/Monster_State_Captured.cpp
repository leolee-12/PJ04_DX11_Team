#include "Monster_State_Captured.h"
#include "Monster.h"
#include "Monster_Movement.h"

HRESULT CMonster_State_Captured::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CMonster_State_Captured::Get_StateType()
{
	return MONSTER_STATE_TYPE::CAPTURED;
}

void CMonster_State_Captured::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->Enable_Controller(false);
	m_pOwner->Enable_Colliders(false);

	if (m_pMovement)
		m_pMovement->Cancle_Launch();		// 기존 넉백/이동량 제거

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);

	m_fPullSpeed = s_fPullInitSpeed;
	m_vBaseScale = m_pOwner->Get_Transform()->Get_Scaled();
	m_fScaleRatio = 1.f;
}

void CMonster_State_Captured::Update(_float fTimeDelta)
{
	if (m_pOwner == nullptr)
		return;

	CGameObject* pCaptor = m_pOwner->Get_Captor();
	if (pCaptor == nullptr)
		return;

	CTransform* pCapT = pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	CTransform* pT = m_pOwner->Get_Transform();
	_vector vSelf = pT->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f) 
	{ 
		m_pOwner->On_Swallowed();
		return; 
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	pT->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vDir) * fMove);

	m_fScaleRatio += (s_fMinScaleRatio - m_fScaleRatio) * min(s_fShrinkLerp * fTimeDelta, 1.f);

	pT->Set_Scale(m_vBaseScale.x * m_fScaleRatio,
		m_vBaseScale.y * m_fScaleRatio,
		m_vBaseScale.z * m_fScaleRatio);
}

void CMonster_State_Captured::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (m_pOwner == nullptr)
		return;

	m_pOwner->Get_Transform()->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);

	if (eNextState != MONSTER_STATE_TYPE::SPAT)
	{
		m_pOwner->Enable_Controller(true);
		m_pOwner->Enable_Colliders(true);
	}
}

CMonster_State_Captured* CMonster_State_Captured::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CMonster_State_Captured* pInstance = new CMonster_State_Captured();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CMonster_State_Captured");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster_State_Captured::Free()
{
	__super::Free();
}
