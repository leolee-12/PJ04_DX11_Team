#include "UI_ClickQTE.h"
#include "UI_Image.h"
#include "GameInstance.h"

namespace
{
    constexpr const _tchar* PART_BUTTON = TEXT("Button");
    constexpr const _tchar* PART_RINGS[3] =
    { TEXT("Ring_0"), TEXT("Ring_1"), TEXT("Ring_2") };
    constexpr _float4 FLASH_COLOR2 = { 0.988f, 0.552f, 0.616f, 1.f };
}

CUI_ClickQTE::CUI_ClickQTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
{
    m_fIdlePeriod = { 0.9f };
    m_fIdleAmp = { 0.08f };
    m_fPunchScale = { 0.25f };
    m_fPunchDecay = { 0.18f };
    m_fRingDuration = { 0.5f };
    m_fRingGrow = { 0.7f };
    m_fFlashDuration = { 0.12f };
}

CUI_ClickQTE::CUI_ClickQTE(const CUI_ClickQTE& Prototype)
    : CUIContainerObject(Prototype)
    , m_fIdlePeriod(Prototype.m_fIdlePeriod)
    , m_fIdleAmp(Prototype.m_fIdleAmp)
    , m_fPunchScale(Prototype.m_fPunchScale)
    , m_fPunchDecay(Prototype.m_fPunchDecay)
    , m_fRingDuration(Prototype.m_fRingDuration)
    , m_fRingGrow(Prototype.m_fRingGrow)
    , m_fFlashDuration(Prototype.m_fFlashDuration)
{
}

HRESULT CUI_ClickQTE::Initialize_Prototype() { return __super::Initialize_Prototype(); }
HRESULT CUI_ClickQTE::Initialize(void* pArg) { return __super::Initialize(pArg); }

HRESULT CUI_ClickQTE::Ready_Events()
{
    Subscribe_Event(EventTag::QTE_Show, [this](void*) { Begin_Prompt(); });
    Subscribe_Event(EventTag::QTE_Hide, [this](void*) { End_Prompt(); });
    return S_OK;
}

void CUI_ClickQTE::Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Update(fTimeDelta);

    if (!m_bRunning)
        return;

    if (m_pGameInstance_Proxy->Mouse_Down(DIMB::LBUTTON))
        On_Click();

    if (m_bFlashing)
    {
        m_fFlashTime += fTimeDelta;
        if (m_fFlashTime >= m_fFlashDuration)
            Restore_Button();
    }

    Tick_Button(fTimeDelta);
    Tick_Ring(fTimeDelta);
}

void CUI_ClickQTE::Begin_Prompt()
{
    Set_Active(true);
    m_bRunning = true;

    m_fPhase = 0.f;
    m_fPunch = 0.f;
    m_iNextRing = 0;
    m_bFlashing = false;

    auto itBtn = m_UIPartObjects.find(PART_BUTTON);
    if (itBtn != m_UIPartObjects.end())
    {
        if (m_fButtonBaseSize <= 0.f)
            m_fButtonBaseSize = XMVectorGetX(XMVector3Length(
                itBtn->second->Get_Transform()->Get_State(STATE::RIGHT)));

        m_vButtonColor2Orig = static_cast<CUI_Image*>(itBtn->second)->Get_Color2();
        itBtn->second->Set_Active(true);
    }

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
}

void CUI_ClickQTE::End_Prompt()
{
    Restore_Button();

    // 버튼 크기를 원래대로 되돌려야 다음 표시 때 어정쩡한 크기로 안 뜸
    auto itBtn = m_UIPartObjects.find(PART_BUTTON);
    if (itBtn != m_UIPartObjects.end() && m_fButtonBaseSize > 0.f)
    {
        CTransform* pTf = itBtn->second->Get_Transform();
        pTf->Set_State(STATE::RIGHT, XMVectorSet(m_fButtonBaseSize, 0.f, 0.f, 0.f));
        pTf->Set_State(STATE::UP, XMVectorSet(0.f, m_fButtonBaseSize, 0.f, 0.f));
    }

    for (_int i = 0; i < RING_COUNT; ++i)
    {
        m_bRingPlaying[i] = false;

        auto it = m_UIPartObjects.find(PART_RINGS[i]);
        if (it != m_UIPartObjects.end())
            it->second->Set_Active(false);
    }

    m_bRunning = false;
    Set_Active(false);
}

void CUI_ClickQTE::On_Click()
{
    // 링 발사
    m_bRingPlaying[m_iNextRing] = true;
    m_fRingTime[m_iNextRing] = 0.f;
    m_iNextRing = (m_iNextRing + 1) % RING_COUNT;

    // 버튼 펀치 + 플래시
    m_fPunch = 1.f;
    m_bFlashing = true;
    m_fFlashTime = 0.f;

    auto itBtn = m_UIPartObjects.find(PART_BUTTON);
    if (itBtn != m_UIPartObjects.end())
        static_cast<CUI_Image*>(itBtn->second)->Set_Color2(FLASH_COLOR2);
}

void CUI_ClickQTE::Tick_Button(_float fTimeDelta)
{
    const _float fPeriod = max(0.05f, m_fIdlePeriod);
    m_fPhase += fTimeDelta / fPeriod;
    if (m_fPhase >= 1.f)
        m_fPhase -= 1.f;

    m_fPunch = max(0.f, m_fPunch - fTimeDelta / max(0.01f, m_fPunchDecay));

    auto itBtn = m_UIPartObjects.find(PART_BUTTON);
    if (itBtn == m_UIPartObjects.end() || m_fButtonBaseSize <= 0.f)
        return;

    // 상시 숨쉬기 + 클릭 펀치(제곱으로 빠르게 빠짐)
    const _float fScale = 1.f
        + m_fIdleAmp * sinf(m_fPhase * XM_2PI)
        + m_fPunchScale * m_fPunch * m_fPunch;

    const _float fSize = m_fButtonBaseSize * fScale;
    CTransform* pTf = itBtn->second->Get_Transform();
    pTf->Set_State(STATE::RIGHT, XMVectorSet(fSize, 0.f, 0.f, 0.f));
    pTf->Set_State(STATE::UP, XMVectorSet(0.f, fSize, 0.f, 0.f));
}

void CUI_ClickQTE::Tick_Ring(_float fTimeDelta)
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

        const _float fSize = m_fRingBaseSize * (1.f + m_fRingGrow * t);
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

void CUI_ClickQTE::Restore_Button()
{
    m_bFlashing = false;

    auto itBtn = m_UIPartObjects.find(PART_BUTTON);
    if (itBtn != m_UIPartObjects.end())
        static_cast<CUI_Image*>(itBtn->second)->Set_Color2(m_vButtonColor2Orig);
}

CUI_ClickQTE* CUI_ClickQTE::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_ClickQTE* pInstance = new CUI_ClickQTE(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_ClickQTE");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_ClickQTE::Clone(void* pArg)
{
    CUI_ClickQTE* pInstance = new CUI_ClickQTE(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_ClickQTE");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_ClickQTE::Free()
{
    __super::Free();
}