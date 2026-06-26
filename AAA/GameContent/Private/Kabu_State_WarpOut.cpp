#include "Kabu_State_WarpOut.h"
#include "Kabu.h"

HRESULT CKabu_State_WarpOut::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CKabu_State_WarpOut::Get_StateType()
{
	return MONSTER_STATE_TYPE::WARPOUT;
}

void CKabu_State_WarpOut::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner)
		return;

	m_fTimer = 0.f;

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CKabu_State_WarpOut::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner || nullptr == m_pAnimator)
		return;

	if (!m_pAnimator->Is_Finished())
		return;


	static_cast<CKabu*>(m_pOwner)->Set_Visible(false);

	m_fTimer += fTimeDelta;

	if (m_fTimer >= s_fWarpInvisibleTime)
		m_pOwner->Change_State(MONSTER_STATE_TYPE::WARPIN);

}

void CKabu_State_WarpOut::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;
}

CKabu_State_WarpOut* CKabu_State_WarpOut::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CKabu_State_WarpOut* pInstance = new CKabu_State_WarpOut();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CKabu_State_WarpOut");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKabu_State_WarpOut::Free()
{
	__super::Free();
}
