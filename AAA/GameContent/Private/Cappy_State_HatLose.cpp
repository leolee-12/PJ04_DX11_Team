#include "Cappy_State_HatLose.h"
#include "Monster.h"
#include "GameInstance.h"

HRESULT CCappy_State_HatLose::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    m_bIsInterruptible = false;   
    return S_OK;
}

MONSTER_STATE_TYPE CCappy_State_HatLose::Get_StateType()
{
    return MONSTER_STATE_TYPE::HAT_LOSE;
}

void CCappy_State_HatLose::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (m_pOwner == nullptr || m_pAnimator == nullptr)
        return;

    ANI_PLAY_INFO Info{};
    Info.strAniName = "HatLose";
    Info.bLoop = false;
    Info.fSpeed = 1.f;
    m_pAnimator->Play(&Info);

    Info.strAniName = "HatLoseLoop";
    Info.bLoop = true;
    Info.fSpeed = 1.f;
    m_pAnimator->Enqueue(Info);     // HatLose 끝나면 Loop로 (무방비 대기)
}

CCappy_State_HatLose* CCappy_State_HatLose::Create(const ANI_PLAY_INFO& tInfo, _float
    fSpeed)
{
    CCappy_State_HatLose* pInstance = new CCappy_State_HatLose();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CCappy_State_HatLose");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCappy_State_HatLose::Free() { __super::Free(); }