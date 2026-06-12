#include "UI_KirbyStatus.h"
#include "UI_GaugeFill.h"
#include "UI_Image.h"

namespace
{
    constexpr const _tchar* COM_GAUGEBAR = TEXT("Com_GaugeBar");

    constexpr const _tchar* PART_HP_FRAME_LEFT = TEXT("Part_HP_Frame_Left");
    constexpr const _tchar* PART_HP_FRAME_CENTER = TEXT("Part_HP_Frame_Center");
    constexpr const _tchar* PART_HP_FRAME_RIGHT = TEXT("Part_HP_Frame_Right");

    constexpr const _tchar* PART_HP_BACK_LEFT = TEXT("Part_HP_Back_Left");
    constexpr const _tchar* PART_HP_BACK_CENTER = TEXT("Part_HP_Back_Center");
    constexpr const _tchar* PART_HP_BACK_RIGHT = TEXT("Part_HP_Back_Right");

    constexpr const _tchar* PART_HP_GHOST_LEFT = TEXT("Part_HP_Ghost_Left");
    constexpr const _tchar* PART_HP_GHOST_CENTER = TEXT("Part_HP_Ghost_Center");
    constexpr const _tchar* PART_HP_GHOST_RIGHT = TEXT("Part_HP_Ghost_Right");

    constexpr const _tchar* PART_HP_HEAL_LEFT = TEXT("Part_HP_Heal_Left");
    constexpr const _tchar* PART_HP_HEAL_CENTER = TEXT("Part_HP_Heal_Center");
    constexpr const _tchar* PART_HP_HEAL_RIGHT = TEXT("Part_HP_Heal_Right");

    constexpr const _tchar* PART_HP_FILL_LEFT = TEXT("Part_HP_Fill_Left");
    constexpr const _tchar* PART_HP_FILL_CENTER = TEXT("Part_HP_Fill_Center");
    constexpr const _tchar* PART_HP_FILL_RIGHT = TEXT("Part_HP_Fill_Right");
}

CUI_KirbyStatus::CUI_KirbyStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject{ pDevice, pContext }
{
}

CUI_KirbyStatus::CUI_KirbyStatus(const CUI_KirbyStatus& Prototype)
    : CUIContainerObject{ Prototype }
    , m_fDefaultMaxHP{ Prototype.m_fDefaultMaxHP }
    , m_fDefaultCurrHP{ Prototype.m_fDefaultCurrHP }
{
}

HRESULT CUI_KirbyStatus::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_KirbyStatus::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CUI_KirbyStatus::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CUI_KirbyStatus::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    if (m_pGaugeBarCom)
        m_pGaugeBarCom->Update(fTimeDelta);

    __super::Update(fTimeDelta);
}

void CUI_KirbyStatus::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_KirbyStatus::Ready_Events()
{
    if (FAILED(__super::Ready_Events()))
        return E_FAIL;

    Subscribe_Event(EventTag::Kirby_HP_Updated,
        [this](void* pData)
        {
            On_HPUpdated(static_cast<KIRBY_HP_UPDATED*>(pData));
        });

    return S_OK;
}

void CUI_KirbyStatus::On_Deserialized()
{
    Bind_GaugeBar();
}

void CUI_KirbyStatus::On_UIPartsChanged()
{
    Bind_GaugeBar();
}

HRESULT CUI_KirbyStatus::Ready_Components()
{
    m_pGaugeBarCom = Add_Component<CUI_GaugeBarCom>(
        COM_GAUGEBAR,
        CUI_GaugeBarCom::Create(m_pDevice, m_pContext));

    if (!m_pGaugeBarCom || FAILED(m_pGaugeBarCom->Initialize(nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CUI_KirbyStatus::Bind_GaugeBar()
{
    if (!m_pGaugeBarCom)
        return E_FAIL;

    auto FindGauge = [this](const _wstring& strTag) -> CUI_GaugeFill*
    {
        auto iter = m_UIPartObjects.find(strTag);
        if (iter == m_UIPartObjects.end())
            return nullptr;

        return dynamic_cast<CUI_GaugeFill*>(iter->second);
    };

    auto FindImage = [this](const _wstring& strTag) -> CUI_Image*
    {
        auto iter = m_UIPartObjects.find(strTag);
        if (iter == m_UIPartObjects.end())
            return nullptr;

        return dynamic_cast<CUI_Image*>(iter->second);
    };

    m_pGaugeBarCom->Clear();

    using LAYER = CUI_GaugeBarCom::LAYER_TYPE;
    using SEGMENTS = CUI_GaugeBarCom::GAUGE_SEGMENTS;
    using FRAME = CUI_GaugeBarCom::FRAME_SEGMENTS;

    auto FailBind = [this]() -> HRESULT
    {
        m_pGaugeBarCom->Clear();
        return E_FAIL;
    };

    if (FAILED(m_pGaugeBarCom->Bind_FrameLayer(
        FRAME{ FindImage(PART_HP_FRAME_LEFT), FindGauge(PART_HP_FRAME_CENTER), FindImage(PART_HP_FRAME_RIGHT) })))
    {
        return FailBind();
    }

    if (FAILED(m_pGaugeBarCom->Bind_Layer(LAYER::BACK,
        SEGMENTS{ FindGauge(PART_HP_BACK_LEFT), FindGauge(PART_HP_BACK_CENTER), FindGauge(PART_HP_BACK_RIGHT) })))
    {
        return FailBind();
    }

    if (FAILED(m_pGaugeBarCom->Bind_Layer(LAYER::GHOST,
        SEGMENTS{ FindGauge(PART_HP_GHOST_LEFT), FindGauge(PART_HP_GHOST_CENTER), FindGauge(PART_HP_GHOST_RIGHT) })))
    {
        return FailBind();
    }

    if (FAILED(m_pGaugeBarCom->Bind_Layer(LAYER::HEAL,
        SEGMENTS{ FindGauge(PART_HP_HEAL_LEFT), FindGauge(PART_HP_HEAL_CENTER), FindGauge(PART_HP_HEAL_RIGHT) })))
    {
        return FailBind();
    }

    if (FAILED(m_pGaugeBarCom->Bind_Layer(LAYER::FILL,
        SEGMENTS{ FindGauge(PART_HP_FILL_LEFT), FindGauge(PART_HP_FILL_CENTER), FindGauge(PART_HP_FILL_RIGHT) })))
    {
        return FailBind();
    }

    Configure_GaugeBar();
    m_pGaugeBarCom->Set_RatioImmediate(m_fDefaultCurrHP / max(1.f, m_fDefaultMaxHP));

    return S_OK;
}

void CUI_KirbyStatus::Configure_GaugeBar()
{
    if (!m_pGaugeBarCom)
        return;

    CUI_GaugeBarCom::GAUGE_LAYOUT_DESC Layout{};
    Layout.fLeftWidth = 18.f;
    Layout.fCenterWidth = 130.f;
    Layout.fRightWidth = 18.f;
    Layout.fHeight = 18.f;
    Layout.fFrameZ = 1.4f;
    Layout.fBackZ = 1.3f;
    Layout.fGhostZ = 1.2f;
    Layout.fHealZ = 1.1f;
    Layout.fFillZ = 1.0f;
    m_pGaugeBarCom->Set_Layout(Layout);

    CUI_GaugeBarCom::GAUGE_MOTION_DESC Motion{};
    Motion.fGhostDelay = 0.35f;
    Motion.fGhostSpeed = 1.8f;
    Motion.fHealSpeed = 2.8f;
    m_pGaugeBarCom->Set_Motion(Motion);

    using LAYER = CUI_GaugeBarCom::LAYER_TYPE;
    m_pGaugeBarCom->Set_LayerColor(LAYER::BACK, _float4(0.03f, 0.03f, 0.03f, 1.f));
    m_pGaugeBarCom->Set_LayerColor(LAYER::GHOST, _float4(1.f, 0.78f, 0.08f, 1.f));
    m_pGaugeBarCom->Set_LayerColor(LAYER::HEAL, _float4(1.f, 0.62f, 0.72f, 0.85f));
    m_pGaugeBarCom->Set_LayerColor(LAYER::FILL, _float4(1.f, 0.12f, 0.42f, 1.f));
}

void CUI_KirbyStatus::On_HPUpdated(const KIRBY_HP_UPDATED* pDesc)
{
    if (!pDesc || !m_pGaugeBarCom)
        return;

    m_pGaugeBarCom->Set_Value(pDesc->fCurrHp, pDesc->fMaxHP);
}

CUI_KirbyStatus* CUI_KirbyStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_KirbyStatus* pInstance = new CUI_KirbyStatus(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_KirbyStatus");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_KirbyStatus::Clone(void* pArg)
{
    CUI_KirbyStatus* pInstance = new CUI_KirbyStatus(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_KirbyStatus");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_KirbyStatus::Free()
{
    __super::Free();
}