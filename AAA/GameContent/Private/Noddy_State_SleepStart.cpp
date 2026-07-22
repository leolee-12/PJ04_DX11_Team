#include "Noddy_State_SleepStart.h"
#include "Monster.h"

HRESULT CNoddy_State_SleepStart::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    m_bIsInterruptible = false;
    return S_OK;
}

MONSTER_STATE_TYPE CNoddy_State_SleepStart::Get_StateType()
{
    return MONSTER_STATE_TYPE::SLEEP_START;
}

void CNoddy_State_SleepStart::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (m_pAnimator && !m_PlayInfo.strAniName.empty())
        m_pAnimator->Play(&m_PlayInfo);
}

void CNoddy_State_SleepStart::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    if (m_pAnimator && m_pAnimator->Is_Finished())
        m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CNoddy_State_SleepStart::Exit(MONSTER_STATE_TYPE eNextState)
{
}

CNoddy_State_SleepStart* CNoddy_State_SleepStart::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CNoddy_State_SleepStart* pInstance = new CNoddy_State_SleepStart();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CNoddy_State_SleepStart");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CNoddy_State_SleepStart::Free()
{
    __super::Free();
}
