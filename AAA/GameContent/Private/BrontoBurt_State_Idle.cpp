#include "BrontoBurt_State_Idle.h"
#include "BrontoBurt.h"
#include "Monster_RailMovement.h"

HRESULT CBrontoBurt_State_Idle::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    m_bIsInterruptible = true;

    return S_OK;
}

MONSTER_STATE_TYPE CBrontoBurt_State_Idle::Get_StateType()
{
    return MONSTER_STATE_TYPE::IDLE;
}

void CBrontoBurt_State_Idle::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (nullptr == m_pOwner)
        return;

    if (m_pAnimator && !m_PlayInfo.strAniName.empty())
        m_pAnimator->Play(&m_PlayInfo);
}

void CBrontoBurt_State_Idle::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner || nullptr == m_pMovement)
        return;

    auto pRail =
        static_cast<CMonster_RailMovement*>(m_pMovement);

    if (pRail->Is_OffPath())
        return;

    pRail->Update_RailFollow(fTimeDelta);
}

void CBrontoBurt_State_Idle::Exit(MONSTER_STATE_TYPE eNextState)
{
    if (nullptr == m_pOwner)
        return;
}

CBrontoBurt_State_Idle* CBrontoBurt_State_Idle::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CBrontoBurt_State_Idle* pInstance = new CBrontoBurt_State_Idle();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CBrontoBurt_State_Idle");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBrontoBurt_State_Idle::Free()
{
    __super::Free();
}
