#include "UI_BossStatus.h"
#include "UI_GaugeFill.h"
#include "UI_GaugeBarCom.h"
#include "UI_Text.h"
#include "GameInstance.h"

namespace
{
    constexpr const _tchar* COM_GAUGEBAR = TEXT("Com_GaugeBar");
    constexpr const _tchar* PART_HP_FILL = TEXT("Guage");   
    constexpr const _tchar* PART_BOSS_NAME = TEXT("BossName");
}

CUI_BossStatus::CUI_BossStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
{
}

CUI_BossStatus::CUI_BossStatus(const CUI_BossStatus& Prototype)
    : CUIContainerObject(Prototype)
    , m_fDefaultMaxHP{ Prototype.m_fDefaultMaxHP }
    , m_fDefaultCurrHP{ Prototype.m_fDefaultCurrHP }
{
}

HRESULT CUI_BossStatus::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_BossStatus::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    Set_Active(false);
    return S_OK;
}

void CUI_BossStatus::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Priority_Update(fTimeDelta);
}

void CUI_BossStatus::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (m_pGaugeBar && !m_pGaugeBar->Is_Bound())
        Try_BindGauge();

    Update_SlideIn(fTimeDelta);

    if (m_bAppearSFXPlaying
        && APPEAR_PHASE::DONE == m_eAppearPhase
        && m_pGaugeBar && m_pGaugeBar->Is_Bound()
        && !m_pGaugeBar->Is_Appearing())
    {
        Stop_AppearSFX();
    }

#ifdef _DEBUG
    if (m_pGaugeBar)
    {
        if (m_pGameInstance_Proxy->Key_Down(DIK_N)) m_pGaugeBar->Add_Value(-10.f);
        if (m_pGameInstance_Proxy->Key_Down(DIK_M)) m_pGaugeBar->Add_Value(+10.f);
    }
#endif

    __super::Update(fTimeDelta);
}

void CUI_BossStatus::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_BossStatus::Ready_Events()
{
    if (FAILED(__super::Ready_Events()))
        return E_FAIL;

    Subscribe_Event(EventTag::Boss_HP_Updated,
        [this](void* pData)
        {
            auto* p = static_cast<BOSS_HP_UPDATED*>(pData);
            if (nullptr == p) return;

            if (m_pGaugeBar && m_pGaugeBar->Is_Bound())
                m_pGaugeBar->Set_Value(p->fCurrHp, p->fMaxHP);
            else
            {
                m_bPendingHP = true;
                m_fPendingMax = p->fMaxHP;
                m_fPendingCurr = p->fCurrHp;
            }
        });

    Subscribe_Event(EventTag::Boss_HP_Appeared,
        [this](void* pData)
        {
            auto* p = static_cast<BOSS_HP_APPEARED*>(pData);
            if (nullptr == p) return;

            Set_Active(true);

            m_strPendingName = p->strBossName;
            m_fPendingMax = p->fMaxHP;
            m_fPendingCurr = p->fCurrHp;

            if (m_pGaugeBar && !m_pGaugeBar->Is_Bound())
                Try_BindGauge();

            if (m_pNameText)
                m_pNameText->Set_Text(p->strBossName);

            if (m_pGaugeBar && m_pGaugeBar->Is_Bound())
                m_pGaugeBar->Reset_Empty();

            Stop_AppearSFX();
            Start_SlideIn();   
        });

    Subscribe_Event(EventTag::Boss_Died, [this](void*) { Stop_AppearSFX(); Set_Active(false); });

    return S_OK;
}

void CUI_BossStatus::Try_BindGauge()
{
    auto it = m_UIPartObjects.find(PART_HP_FILL);
    if (it == m_UIPartObjects.end())
        return;

    m_pGaugeBar->Bind_Gauge(static_cast<CUI_GaugeFill*>(it->second));

    if (nullptr == m_pNameText)
    {
        auto itName = m_UIPartObjects.find(PART_BOSS_NAME);
        if (itName != m_UIPartObjects.end())
            m_pNameText = dynamic_cast<CUI_Text*>(itName->second);
    }
    if (m_pNameText && !m_strPendingName.empty())
        m_pNameText->Set_Text(m_strPendingName);

    if (m_bPendingAppear)
    {
        m_pGaugeBar->Appear(m_fPendingCurr, m_fPendingMax);
        Start_AppearSFX();
        m_bPendingAppear = false;
        m_bPendingHP = false;
    }
    else if (m_bPendingHP)
    {
        m_pGaugeBar->Set_Value(m_fPendingCurr, m_fPendingMax);
        m_bPendingHP = false;
    }
}

void CUI_BossStatus::Start_AppearSFX()
{
    m_hAppearSFX = m_pGameInstance_Proxy->Play_SFX(L"UiBasic_BossHpUp.wav", 0.35f, ESoundBus::UI);
    m_bAppearSFXPlaying = true;
}

void CUI_BossStatus::Stop_AppearSFX()
{
    m_hAppearSFX.Stop();
    m_bAppearSFXPlaying = false;
}

void CUI_BossStatus::Start_SlideIn()
{
    _vector vHome = m_pTransformCom->Get_State(STATE::POSITION);
    XMStoreFloat3(&m_vHomePos, vHome);

    _vector vStart = vHome + XMVectorSet(m_fSlideOffsetX, 0.f, 0.f, 0.f);
    m_pTransformCom->Set_State(STATE::POSITION, vStart);

    m_fSlideTime = 0.f;
    m_eAppearPhase = APPEAR_PHASE::SLIDING;
}

void CUI_BossStatus::Update_SlideIn(_float fTimeDelta)
{
    if (m_eAppearPhase != APPEAR_PHASE::SLIDING)
        return;

    m_fSlideTime += fTimeDelta;
    _float t = (m_fSlideDuration > 0.f) ? (m_fSlideTime / m_fSlideDuration) : 1.f;
    if (t > 1.f) t = 1.f;

    _float fStartX = m_vHomePos.x + m_fSlideOffsetX;
    _float fX = fStartX + (m_vHomePos.x - fStartX) * Ease_OutCubic(t);

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetX(vPos, fX));

    if (t >= 1.f)
    {
        m_pTransformCom->Set_State(STATE::POSITION,
            XMVectorSet(m_vHomePos.x, m_vHomePos.y, m_vHomePos.z, 1.f));
        m_eAppearPhase = APPEAR_PHASE::DONE;

        if (m_pGaugeBar && m_pGaugeBar->Is_Bound())
        {
            m_pGaugeBar->Appear(m_fPendingCurr, m_fPendingMax);
            Start_AppearSFX();
        }
        else
            m_bPendingAppear = true;  
    }
}

HRESULT CUI_BossStatus::Ready_Components()
{
    m_pGaugeBar = Add_Component<CUI_GaugeBarCom>(
        COM_GAUGEBAR, CUI_GaugeBarCom::Create(m_pDevice, m_pContext));
    if (!m_pGaugeBar)
        return E_FAIL;

    return S_OK;
}

CUI_BossStatus* CUI_BossStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_BossStatus* pInstance = new CUI_BossStatus(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_BossStatus");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_BossStatus::Clone(void* pArg)
{
    CUI_BossStatus* pInstance = new CUI_BossStatus(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_BossStatus");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_BossStatus::Free()
{
    Stop_AppearSFX();
    __super::Free();
}