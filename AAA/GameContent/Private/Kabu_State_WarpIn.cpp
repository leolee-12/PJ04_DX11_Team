#include "Kabu_State_WarpIn.h"
#include "Kabu.h"
#include "Monster_RailMovement.h"

HRESULT CKabu_State_WarpIn::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CKabu_State_WarpIn::Get_StateType()
{
	return MONSTER_STATE_TYPE::WARPIN;
}

void CKabu_State_WarpIn::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner)
		return;

    if (m_pMovement)
        static_cast<CMonster_RailMovement*>(m_pMovement)->Warp_ToRandomPoint();

    static_cast<CKabu*>(m_pOwner)->Set_Visible(true);

    if (m_pAnimator && !m_PlayInfo.strAniName.empty())
        m_pAnimator->Play(&m_PlayInfo);
}

void CKabu_State_WarpIn::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner || nullptr == m_pAnimator)
        return;

    if (!m_pAnimator->Is_Finished())
        return;

    if (m_pAnimator->Is_Finished())
        m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CKabu_State_WarpIn::Exit(MONSTER_STATE_TYPE eNextState)
{
    if (nullptr == m_pOwner)
        return;
}

CKabu_State_WarpIn* CKabu_State_WarpIn::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CKabu_State_WarpIn* pInstance = new CKabu_State_WarpIn();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CKabu_State_WarpIn");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CKabu_State_WarpIn::Free()
{
    __super::Free();
}
