#include "UI_QTE.h"
#include "UI_Image.h"
#include "GameInstance.h"

namespace
{
    constexpr const _tchar* PART_STICKS[5] =
    { TEXT("Stick_C"), TEXT("Stick_L"), TEXT("Stick_R"), TEXT("Stick_U"), TEXT("Stick_D") };
    constexpr const _tchar* PART_RINGS[3] =
    { TEXT("Ring_0"), TEXT("Ring_1"), TEXT("Ring_2") };
    constexpr _float4 FLASH_COLOR2 = { 0.988f, 0.552f, 0.616f, 1.f };
}

CUI_QTE::CUI_QTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
{
    m_fBaseInterval = { 0.45f };
    m_fFastInterval = { 0.1f };
    m_fHeatPerInput = { 0.34f };
    m_fHeatDecay = { 1.2f };
    m_fRingDuration = { 0.4f };
    m_fFlashDuration = { 0.12f };
}

CUI_QTE::CUI_QTE(const CUI_QTE& Prototype)
    : CUIContainerObject(Prototype)
    , m_fBaseInterval(Prototype.m_fBaseInterval)
    , m_fFastInterval(Prototype.m_fFastInterval)
    , m_fHeatPerInput(Prototype.m_fHeatPerInput)
    , m_fHeatDecay(Prototype.m_fHeatDecay)
    , m_fRingDuration(Prototype.m_fRingDuration)
    , m_fFlashDuration(Prototype.m_fFlashDuration)
{
}

HRESULT CUI_QTE::Initialize_Prototype() { return __super::Initialize_Prototype(); }
HRESULT CUI_QTE::Initialize(void* pArg) { return __super::Initialize(pArg); }

void CUI_QTE::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Priority_Update(fTimeDelta);
}

void CUI_QTE::Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Update(fTimeDelta);

    if (!m_bRunning)
        return;

    // 입력 펄스 (PC: WASD)
    if (m_pGameInstance_Proxy->Key_Down(DIK_W) || m_pGameInstance_Proxy->Key_Down(DIK_A) ||
        m_pGameInstance_Proxy->Key_Down(DIK_S) || m_pGameInstance_Proxy->Key_Down(DIK_D))
        On_Pulse();

    // 열 감쇠
    m_fHeat = max(0.f, m_fHeat - m_fHeatDecay * fTimeDelta);

    // 열에 따라 전환 간격 보간
    const _float fInterval = m_fBaseInterval + (m_fFastInterval - m_fBaseInterval) * m_fHeat;
    m_fSwitchTimer += fTimeDelta;
    if (m_fSwitchTimer >= fInterval)
    {
        m_fSwitchTimer = 0.f;
        Switch_Stick();
    }

    if (m_bFlashing)
    {
        m_fFlashTime += fTimeDelta;
        if (m_fFlashTime >= m_fFlashDuration)
            Restore_StickColor();
    }
    Tick_Ring(fTimeDelta);
}

void CUI_QTE::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_QTE::Ready_Events()
{
    Subscribe_Event(EventTag::QTE_Show, [this](void*) { Begin_QTE(); });
    Subscribe_Event(EventTag::QTE_Hide, [this](void*) 
        { 
            if(m_bFlashing)
                Restore_StickColor();
            m_bRunning = false; 
            Set_Active(false); 
        });
    return S_OK;
}

void CUI_QTE::Begin_QTE()
{
    Set_Active(true);
    m_bRunning = true;

    m_fHeat = 0.f;
    m_fSwitchTimer = 0.f;
    for (_int i = 0; i < RING_COUNT; ++i)
    {
        m_bRingPlaying[i] = false;

        auto it = m_UIPartObjects.find(PART_RINGS[i]);
        if (it != m_UIPartObjects.end())
        {
            if (m_fRingBaseSize <= 0.f)
                m_fRingBaseSize = XMVectorGetX(XMVector3Length(
                    it->second->Get_Transform()->Get_State(STATE::RIGHT)));
            it->second->Set_Active(false);
        }
    }
    m_iNextRing = 0;

    m_bFlashing = false;
    for (_int i = 0; i < STICK_COUNT; ++i)
    {
        auto it = m_UIPartObjects.find(PART_STICKS[i]);
        if (it != m_UIPartObjects.end())
            m_vStickColor2Orig[i] = static_cast<CUI_Image*>(it->second)->Get_Color2();
    }

    Show_Stick(0);      // 가운데부터 시작
}

void CUI_QTE::On_Pulse()
{
    m_fHeat = min(1.f, m_fHeat + m_fHeatPerInput);

    m_bRingPlaying[m_iNextRing] = true;
    m_fRingTime[m_iNextRing] = 0.f;
    m_iNextRing = (m_iNextRing + 1) % RING_COUNT;

    m_bFlashing = true;
    m_fFlashTime = 0.f;
    for (_int i = 0; i < STICK_COUNT; ++i)
    {
        auto it = m_UIPartObjects.find(PART_STICKS[i]);
        if (it != m_UIPartObjects.end())
            static_cast<CUI_Image*>(it->second)->Set_Color2(FLASH_COLOR2);
    }
}

void CUI_QTE::Switch_Stick()
{
    _int iNext = m_pGameInstance_Proxy->RandomInt(0, STICK_COUNT - 1);
    if (iNext == m_iCurStick)
        iNext = (iNext + 1) % STICK_COUNT;

    Show_Stick(iNext);
}

void CUI_QTE::Show_Stick(_int iIndex)
{
    m_iCurStick = iIndex;

    for (_int i = 0; i < STICK_COUNT; ++i)
    {
        auto it = m_UIPartObjects.find(PART_STICKS[i]);
        if (it != m_UIPartObjects.end())
            it->second->Set_Active(i == iIndex);
    }
}

void CUI_QTE::Tick_Ring(_float fTimeDelta)
{
    for (_int i = 0; i < RING_COUNT; ++i)
    {
        if (!m_bRingPlaying[i])
            continue;

        auto it = m_UIPartObjects.find(PART_RINGS[i]);
        if (it == m_UIPartObjects.end())
        {
            m_bRingPlaying[i] = false;
            continue;
        }

        m_fRingTime[i] += fTimeDelta;
        const _float t = min(1.f, m_fRingTime[i] / max(0.001f, m_fRingDuration));

        const _float fSize = m_fRingBaseSize * (1.f + 0.7f * t);
        CTransform* pTf = it->second->Get_Transform();
        pTf->Set_State(STATE::RIGHT, XMVectorSet(fSize, 0.f, 0.f, 0.f));
        pTf->Set_State(STATE::UP, XMVectorSet(0.f, fSize, 0.f, 0.f));

        static_cast<CUI_Image*>(it->second)->Set_Alpha(1.f - t);
        it->second->Set_Active(true);

        if (t >= 1.f)
        {
            m_bRingPlaying[i] = false;
            it->second->Set_Active(false);
        }
    }
}

void CUI_QTE::Restore_StickColor()
{
    m_bFlashing = false;
    for (_int i = 0; i < STICK_COUNT; ++i)
    {
        auto it = m_UIPartObjects.find(PART_STICKS[i]);
        if (it != m_UIPartObjects.end())
            static_cast<CUI_Image*>(it->second)->Set_Color2(m_vStickColor2Orig[i]);
    }
}

CUI_QTE* CUI_QTE::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_QTE* pInstance = new CUI_QTE(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_QTE");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_QTE::Clone(void* pArg)
{
    CUI_QTE* pInstance = new CUI_QTE(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_QTE");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_QTE::Free()
{
    __super::Free();
}