#include "Cappy_State_Detect.h"
#include "GameInstance.h"
#include "Monster.h"

HRESULT CCappy_State_Detect::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    if (FAILED(__super::Initialize(tInfo, fSpeed)))
        return E_FAIL;

    m_bIsInterruptible = false;

    return S_OK;
}

MONSTER_STATE_TYPE CCappy_State_Detect::Get_StateType()
{
    return MONSTER_STATE_TYPE::DETECT;
}

void CCappy_State_Detect::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (m_pOwner == nullptr || m_pAnimator == nullptr)
        return;

    Play_DetectAnimation();
}

void CCappy_State_Detect::Update(_float fTimeDelta)
{
    if (m_pOwner == nullptr || m_pAnimator == nullptr)
        return;

    __super::Update(fTimeDelta);
}

void CCappy_State_Detect::Exit(MONSTER_STATE_TYPE eNextState)
{
    if (m_pOwner == nullptr)
        return;
}

void CCappy_State_Detect::Play_DetectAnimation()
{
    if (nullptr == m_pOwner || m_pAnimator == nullptr)
        return;

    _int iRand = rand() % 2;

    ANI_PLAY_INFO		Info{};
    LAYER_PLAY_INFO		LayerInfo{};

    switch (iRand)
    {
    case 0:
    {
        Info.strAniName = "KasaUp1";
        Info.bLoop = false;
        Info.fSpeed = 1.f;

        m_pAnimator->Play(&Info);
        break;
    }
    case 1:
    {
        Info.strAniName = "KasaUp2";
        Info.bLoop = false;
        Info.fSpeed = 1.f;

        m_pAnimator->Play(&Info);
        break;
    }
    }
}

CCappy_State_Detect* CCappy_State_Detect::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CCappy_State_Detect* pInstance = new CCappy_State_Detect();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CCappy_State_Detect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCappy_State_Detect::Free()
{
    __super::Free();
}
