#include "Cappy_State_Detect.h"
#include "GameInstance.h"

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

    ANI_PLAY_INFO Info{};

    Info.strAniName = "KasaUp3";
    Info.bLoop = false;
    Info.fSpeed = 1.25f;
    
    m_pAnimator->Play(&Info);

    LAYER_PLAY_INFO LayerInfo{};
    LayerInfo.iSlot = 1;
    LayerInfo.tAnim.strAniName = "KasaUp1";
    LayerInfo.tAnim.bLoop = false;
    LayerInfo.tAnim.fSpeed = 1.25f;
    LayerInfo.Roots = { "CappyHatL" };

    m_pAnimator->Apply_Overlay(LayerInfo);
}

void CCappy_State_Detect::Update(_float fTimeDelta)
{
    if (m_pOwner == nullptr)
        return;

    __super::Update(fTimeDelta);
}

void CCappy_State_Detect::Exit(MONSTER_STATE_TYPE eNextState)
{
    if (m_pOwner == nullptr || m_pAnimator == nullptr)
        return;

    if (m_pAnimator)
    {
        m_pAnimator->Clear_Overlay(1);
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
