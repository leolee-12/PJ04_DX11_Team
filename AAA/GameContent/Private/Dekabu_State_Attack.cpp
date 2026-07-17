#include "Dekabu_State_Attack.h"
#include "Monster.h"

HRESULT CDekabu_State_Attack::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	m_eNextState = MONSTER_STATE_TYPE::WINDUP;
	return S_OK;
}

MONSTER_STATE_TYPE CDekabu_State_Attack::Get_StateType()
{
	return MONSTER_STATE_TYPE::ATTACK;
}

void CDekabu_State_Attack::Enter(MONSTER_STATE_TYPE ePrevState)
{
    UNREFERENCED_PARAMETER(ePrevState);

    if (nullptr == m_pOwner || nullptr == m_pAnimator)
        return;

    ANI_PLAY_INFO Info{};
    Info.strAniName = "Attack";
    Info.bLoop = false;
    Info.fSpeed = 1.0f;
    m_pAnimator->Play(&Info);
}

void CDekabu_State_Attack::Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (nullptr == m_pOwner || nullptr == m_pAnimator)
        return;

    if (m_pAnimator->Is_Finished())
    {
        const MONSTER_BLACKBOARD& BB = m_pOwner->Get_BlackBoard();
        m_pOwner->Change_State((nullptr != BB.pTarget)
            ? MONSTER_STATE_TYPE::WINDUP
            : MONSTER_STATE_TYPE::IDLE);
    }
}

void CDekabu_State_Attack::Exit(MONSTER_STATE_TYPE eNextState)
{
    UNREFERENCED_PARAMETER(eNextState);
}

CDekabu_State_Attack* CDekabu_State_Attack::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CDekabu_State_Attack* pInstance = new CDekabu_State_Attack();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CDekabu_State_Attack");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDekabu_State_Attack::Free()
{
    __super::Free();
}
