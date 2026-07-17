#include "Dekabu_State_Alert.h"
#include "Monster.h"

HRESULT CDekabu_State_Alert::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_eNextState = MONSTER_STATE_TYPE::ATTACK;
	return S_OK;
}

MONSTER_STATE_TYPE CDekabu_State_Alert::Get_StateType()
{
	return MONSTER_STATE_TYPE::WINDUP;
}

void CDekabu_State_Alert::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (nullptr == m_pOwner || nullptr == m_pAnimator)
        return;

    m_fMaxTime = (ePrevState == MONSTER_STATE_TYPE::ATTACK)
        ? 2.0f : 0.8f;

    ANI_PLAY_INFO Info{};
    Info.strAniName = "AttackWait";
    Info.bLoop = true;
    Info.fSpeed = 1.0f;
    m_pAnimator->Play(&Info);
}

void CDekabu_State_Alert::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();

    if (nullptr == BB.pTarget)
    {
        m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
        return;
    }

    m_fMaxTime -= fTimeDelta;
    if (m_fMaxTime <= 0.f)
        m_pOwner->Change_State(m_eNextState);
}

void CDekabu_State_Alert::Exit(MONSTER_STATE_TYPE eNextState)
{
    UNREFERENCED_PARAMETER(eNextState);
}

CDekabu_State_Alert* CDekabu_State_Alert::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CDekabu_State_Alert* pInstance = new CDekabu_State_Alert();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CDekabu_State_Alert");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDekabu_State_Alert::Free()
{
    __super::Free();
}
