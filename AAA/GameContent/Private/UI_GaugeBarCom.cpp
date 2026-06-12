#include "UI_GaugeBarCom.h"
#include "UI_GaugeFill.h"
#include "UI_Image.h"
#include "Transform.h"

CUI_GaugeBarCom::CUI_GaugeBarCom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
{
}

CUI_GaugeBarCom::CUI_GaugeBarCom(const CUI_GaugeBarCom& Prototype)
    : CComponent(Prototype )
{
}

HRESULT CUI_GaugeBarCom::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_GaugeBarCom::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

void CUI_GaugeBarCom::Update(_float fTimeDelta)
{
    Update_Ghost(fTimeDelta);
    Update_Heal(fTimeDelta);
}

void CUI_GaugeBarCom::Clear()
{
    m_FrameLayer = {};

    for (_uint i = 0; i < ETOUI(LAYER_TYPE::END); ++i)
        m_Layers[i] = {};

    m_bInitializedRatio = false;
    m_fTargetRatio = 1.f;
    m_fFillRatio = 1.f;
    m_bGhostPlaying = false;
    m_fGhostRatio = 1.f;
    m_fGhostDelayAcc = 0.f;
    m_bHealPlaying = false;
    m_fHealRatio = 1.f;
}

HRESULT CUI_GaugeBarCom::Bind_FrameLayer(const FRAME_SEGMENTS& Segments)
{
    if (!Segments.pLeft || !Segments.pCenter || !Segments.pRight)
        return E_FAIL;

    m_FrameLayer.Segments = Segments;
    m_FrameLayer.bBound = true;

    m_FrameLayer.Segments.pCenter->Set_FillRatio(1.f);

    return S_OK;
}

HRESULT CUI_GaugeBarCom::Bind_Layer(LAYER_TYPE eLayer, const GAUGE_SEGMENTS& Segments)
{
    GAUGE_LAYER* pLayer = Get_Layer(eLayer);
    if (!pLayer)
        return E_FAIL;

    if (!Segments.pLeft || !Segments.pCenter || !Segments.pRight)
        return E_FAIL;

    pLayer->Segments = Segments;
    pLayer->bBound = true;

    Apply_LayerColor(*pLayer);
    Apply_LayerRatio(*pLayer, eLayer == LAYER_TYPE::BACK ? 1.f : m_fTargetRatio);

    if (eLayer == LAYER_TYPE::GHOST || eLayer == LAYER_TYPE::HEAL)
        Set_LayerActive(*pLayer, false);

    return S_OK;
}

void CUI_GaugeBarCom::Set_Layout(const GAUGE_LAYOUT_DESC& Desc)
{
    m_Layout = Desc;

    m_Layout.fLeftWidth = max(0.0001f, m_Layout.fLeftWidth);
    m_Layout.fCenterWidth = max(0.0001f, m_Layout.fCenterWidth);
    m_Layout.fRightWidth = max(0.0001f, m_Layout.fRightWidth);
    m_Layout.fHeight = max(0.0001f, m_Layout.fHeight);

    Apply_FrameLayout(m_Layout.fFrameZ);
    Apply_LayoutToLayer(m_Layers[ETOI(LAYER_TYPE::BACK)], m_Layout.fBackZ);
    Apply_LayoutToLayer(m_Layers[ETOI(LAYER_TYPE::GHOST)], m_Layout.fGhostZ);
    Apply_LayoutToLayer(m_Layers[ETOI(LAYER_TYPE::HEAL)], m_Layout.fHealZ);
    Apply_LayoutToLayer(m_Layers[ETOI(LAYER_TYPE::FILL)], m_Layout.fFillZ);
}

void CUI_GaugeBarCom::Set_Motion(const GAUGE_MOTION_DESC& Desc)
{
    m_Motion = Desc;
    m_Motion.fGhostDelay = max(0.f, m_Motion.fGhostDelay);
    m_Motion.fGhostSpeed = max(0.01f, m_Motion.fGhostSpeed);
    m_Motion.fHealSpeed = max(0.01f, m_Motion.fHealSpeed);
}

void CUI_GaugeBarCom::Set_LayerColor(LAYER_TYPE eLayer, const _float4& vColor)
{
    GAUGE_LAYER* pLayer = Get_Layer(eLayer);
    if (!pLayer)
        return;

    pLayer->vColor = vColor;
    Apply_LayerColor(*pLayer);
}

void CUI_GaugeBarCom::Set_RatioImmediate(_float fRatio)
{
    fRatio = clamp(fRatio, 0.f, 1.f);

    m_bInitializedRatio = true;
    m_fTargetRatio = fRatio;
    m_fFillRatio = fRatio;
    m_fGhostRatio = fRatio;
    m_fHealRatio = fRatio;

    m_bGhostPlaying = false;
    m_bHealPlaying = false;
    m_fGhostDelayAcc = 0.f;

    Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::BACK)], 1.f);
    Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::GHOST)], fRatio);
    Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::HEAL)], fRatio);
    Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::FILL)], fRatio);

    Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::GHOST)], false);
    Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::HEAL)], false);
}

void CUI_GaugeBarCom::Set_Value(_float fCurrent, _float fMax)
{
    const _float fNewRatio = clamp((fCurrent / max(1.f, fMax)), 0.f, 1.f);

    if (!m_bInitializedRatio)
    {
        Set_RatioImmediate(fNewRatio);
        return;
    }

    const _float fPrevVisibleRatio = m_fFillRatio;
    m_fTargetRatio = fNewRatio;

    if (fNewRatio < fPrevVisibleRatio)
    {
        m_bHealPlaying = false;
        Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::HEAL)], false);

        m_fFillRatio = fNewRatio;
        Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::FILL)], m_fFillRatio);

        m_fGhostRatio = fPrevVisibleRatio;
        m_fGhostDelayAcc = 0.f;
        m_bGhostPlaying = true;

        Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::GHOST)], true);
        Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::GHOST)], m_fGhostRatio);
    }
    else if (fNewRatio > fPrevVisibleRatio)
    {
        m_bGhostPlaying = false;
        Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::GHOST)], false);

        m_fHealRatio = fNewRatio;
        m_bHealPlaying = true;

        Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::HEAL)], true);
        Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::HEAL)], m_fHealRatio);
    }
}

CUI_GaugeBarCom::GAUGE_LAYER* CUI_GaugeBarCom::Get_Layer(LAYER_TYPE eLayer)
{
    const _uint iIndex = ETOUI(eLayer);
    if (iIndex >= ETOUI(LAYER_TYPE::END))
        return nullptr;

    return &m_Layers[iIndex];
}

CUI_GaugeBarCom::SEGMENT_RATIO CUI_GaugeBarCom::Compute_SegmentRatio(_float fRatio) const
{
    fRatio = clamp(fRatio, 0.f, 1.f);

    const _float fTotal = m_Layout.fLeftWidth + m_Layout.fCenterWidth + m_Layout.fRightWidth;
    const _float fVisible = fTotal * fRatio;

    SEGMENT_RATIO Result{};
    Result.fLeft = clamp((fVisible / m_Layout.fLeftWidth), 0.f, 1.f );
    Result.fCenter = clamp(((fVisible - m_Layout.fLeftWidth) / m_Layout.fCenterWidth), 0.f, 1.f);
    Result.fRight = clamp(((fVisible - m_Layout.fLeftWidth - m_Layout.fCenterWidth) / m_Layout.fRightWidth), 0.f, 1.f);

    return Result;
}

void CUI_GaugeBarCom::Apply_LayerRatio(GAUGE_LAYER& Layer, _float fRatio)
{
    if (!Layer.bBound)
        return;

    const SEGMENT_RATIO Ratio = Compute_SegmentRatio(fRatio);

    Layer.Segments.pLeft->Set_FillRatio(Ratio.fLeft);
    Layer.Segments.pCenter->Set_FillRatio(Ratio.fCenter);
    Layer.Segments.pRight->Set_FillRatio(Ratio.fRight);
}

void CUI_GaugeBarCom::Apply_LayerColor(GAUGE_LAYER& Layer)
{
    if (!Layer.bBound)
        return;

    Layer.Segments.pLeft->Set_Color(Layer.vColor);
    Layer.Segments.pCenter->Set_Color(Layer.vColor);
    Layer.Segments.pRight->Set_Color(Layer.vColor);
}

void CUI_GaugeBarCom::Set_LayerActive(GAUGE_LAYER& Layer, _bool bActive)
{
    if (!Layer.bBound)
        return;

    Layer.Segments.pLeft->Set_Active(bActive);
    Layer.Segments.pCenter->Set_Active(bActive);
    Layer.Segments.pRight->Set_Active(bActive);
}

void CUI_GaugeBarCom::Apply_FrameLayout(_float fZ)
{
    if (!m_FrameLayer.bBound)
        return;

    const _float fTotal = m_Layout.fLeftWidth + m_Layout.fCenterWidth + m_Layout.fRightWidth;
    const _float fLeftX = -fTotal * 0.5f + m_Layout.fLeftWidth * 0.5f;
    const _float fCenterX = -fTotal * 0.5f + m_Layout.fLeftWidth + m_Layout.fCenterWidth * 0.5f;
    const _float fRightX = fTotal * 0.5f - m_Layout.fRightWidth * 0.5f;

    auto Apply = [this] (CUIPartObject* pPart, _float fX, _float fWidth, _float fZ)
    {
        if (!pPart || !pPart->Get_Transform())
            return;

        pPart->Get_Transform()->Set_Scale(fWidth, m_Layout.fHeight, 1.f);
        pPart->Get_Transform()->Set_State(
            STATE::POSITION,
            XMVectorSet(fX, 0.f, fZ, 1.f));
    };

    Apply(m_FrameLayer.Segments.pLeft, fLeftX, m_Layout.fLeftWidth, fZ);
    Apply(m_FrameLayer.Segments.pCenter, fCenterX, m_Layout.fCenterWidth, fZ);
    Apply(m_FrameLayer.Segments.pRight, fRightX, m_Layout.fRightWidth, fZ);
}
void CUI_GaugeBarCom::Apply_LayoutToLayer(GAUGE_LAYER& Layer, _float fZ)
{
    if (!Layer.bBound)
        return;

    const _float fTotal = m_Layout.fLeftWidth + m_Layout.fCenterWidth + m_Layout.fRightWidth;
    const _float fLeftX = -fTotal * 0.5f + m_Layout.fLeftWidth * 0.5f;
    const _float fCenterX = -fTotal * 0.5f + m_Layout.fLeftWidth + m_Layout.fCenterWidth * 0.5f;
    const _float fRightX = fTotal * 0.5f - m_Layout.fRightWidth * 0.5f;

    auto Apply = [this] (CUI_GaugeFill* pPart, _float fX, _float fWidth, _float fZ)
    {
        if (!pPart || !pPart->Get_Transform())
            return;

        pPart->Get_Transform()->Set_Scale(fWidth, m_Layout.fHeight, 1.f);
        pPart->Get_Transform()->Set_State(
            STATE::POSITION,
            XMVectorSet(fX, 0.f, fZ, 1.f));
    };

    Apply(Layer.Segments.pLeft, fLeftX, m_Layout.fLeftWidth, fZ);
    Apply(Layer.Segments.pCenter, fCenterX, m_Layout.fCenterWidth, fZ);
    Apply(Layer.Segments.pRight, fRightX, m_Layout.fRightWidth, fZ);
}

void CUI_GaugeBarCom::Update_Ghost(_float fTimeDelta)
{
    if (!m_bGhostPlaying)
        return;

    m_fGhostDelayAcc += fTimeDelta;
    if (m_fGhostDelayAcc < m_Motion.fGhostDelay)
        return;

    m_fGhostRatio -= m_Motion.fGhostSpeed * fTimeDelta;

    if (m_fGhostRatio <= m_fFillRatio)
    {
        m_fGhostRatio = m_fFillRatio;
        m_bGhostPlaying = false;
        Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::GHOST)], false);
    }

    Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::GHOST)], m_fGhostRatio);
}

void CUI_GaugeBarCom::Update_Heal(_float fTimeDelta)
{
    if (!m_bHealPlaying)
        return;

    m_fFillRatio += m_Motion.fHealSpeed * fTimeDelta;

    if (m_fFillRatio >= m_fTargetRatio)
    {
        m_fFillRatio = m_fTargetRatio;
        m_bHealPlaying = false;
        Set_LayerActive(m_Layers[ETOI(LAYER_TYPE::HEAL)], false);
    }

    Apply_LayerRatio(m_Layers[ETOI(LAYER_TYPE::FILL)], m_fFillRatio);
}

CUI_GaugeBarCom* CUI_GaugeBarCom::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_GaugeBarCom* pInstance = new CUI_GaugeBarCom(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_GaugeBarCom");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CUI_GaugeBarCom::Clone(void* pArg)
{
    CUI_GaugeBarCom* pInstance = new CUI_GaugeBarCom(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_GaugeBarCom");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_GaugeBarCom::Free()
{
    __super::Free();
}
