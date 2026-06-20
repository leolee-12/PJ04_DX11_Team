#include "Monster_State_Captured.h"
#include "Monster.h"

HRESULT CMonster_State_Captured::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	return S_OK;
}

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

void CMonster_State_Captured::On_Enter(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	pMonster->Enable_Controller(false);
	pMonster->Enable_Colliders(false);

	if (!m_PlayInfo.strAniName.empty())
		if (CAnimator* pAnim = pMonster->Get_BodyAnimator())
			pAnim->Play(&m_PlayInfo);

	m_fPullSpeed = s_fPullInitSpeed;
	m_vBaseScale = pMonster->Get_Transform()->Get_Scaled();
	m_fScaleRatio = 1.f;
}

void CMonster_State_Captured::On_Update(CMonster* pMonster, _float fTimeDelta)
{
	if (pMonster == nullptr)
		return;

	CGameObject* pCaptor = pMonster->Get_Captor();
	if (pCaptor == nullptr)
		return;

	CTransform* pCapT = pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	CTransform* pT = pMonster->Get_Transform();
	_vector vSelf = pT->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f) 
	{ 
		pMonster->On_Swallowed();
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

void CMonster_State_Captured::On_Exit(CMonster* pMonster)
{
	if (pMonster == nullptr)
		return;

	pMonster->Get_Transform()->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);
	pMonster->Enable_Controller(true);
	pMonster->Enable_Colliders(true);
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

CMonster_State_Captured* CMonster_State_Captured::Create()
{
	CMonster_State_Captured* pInstance = new CMonster_State_Captured();
	if (FAILED(pInstance->Initialize()))
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
