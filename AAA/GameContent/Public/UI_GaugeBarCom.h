#pragma once

#include "GameContent_Defines.h"
#include "Component.h"
#include "UI_Enums.h"
#include "UI_Structs.h"

NS_BEGIN(Client)

class CUI_GaugeFill;
class CUI_Image;

class CLIENT_DLL CUI_GaugeBarCom final : public CComponent
{
public:
    using LAYER_TYPE = Client::UI_GAUGE_LAYER_TYPE;
    using GAUGE_SEGMENTS = Client::UI_GAUGE_SEGMENTS;
    using FRAME_SEGMENTS = Client::UI_FRAME_SEGMENTS;
    using GAUGE_LAYOUT_DESC = Client::UI_GAUGE_LAYOUT_DESC;
    using GAUGE_MOTION_DESC = Client::UI_GAUGE_MOTION_DESC;

private:
    using GAUGE_LAYER = Client::UI_GAUGE_LAYER;
    using SEGMENT_RATIO = Client::UI_GAUGE_SEGMENT_RATIO;
    using FRAME_LAYER = Client::UI_FRAME_LAYER;

public:
    CUI_GaugeBarCom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_GaugeBarCom(const CUI_GaugeBarCom& Prototype);
    virtual ~CUI_GaugeBarCom() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    void Update(_float fTimeDelta);

public:
    void Clear();

    HRESULT Bind_FrameLayer(const FRAME_SEGMENTS& Segments);
    HRESULT Bind_Layer(LAYER_TYPE eLayer, const GAUGE_SEGMENTS& Segments);
    void Set_Layout(const GAUGE_LAYOUT_DESC& Desc);
    void Set_Motion(const GAUGE_MOTION_DESC& Desc);

    void Set_LayerColor(LAYER_TYPE eLayer, const _float4& vColor);
    void Set_RatioImmediate(_float fRatio);
    void Set_Value(_float fCurrent, _float fMax);

private:
    GAUGE_LAYER* Get_Layer(LAYER_TYPE eLayer);

    SEGMENT_RATIO Compute_SegmentRatio(_float fRatio) const;
    void Apply_LayerRatio(GAUGE_LAYER& Layer, _float fRatio);
    void Apply_LayerColor(GAUGE_LAYER& Layer);
    void Set_LayerActive(GAUGE_LAYER& Layer, _bool bActive);

    void Apply_FrameLayout(_float fZ);
    void Apply_LayoutToLayer(GAUGE_LAYER& Layer, _float fZ);
    void Update_Ghost(_float fTimeDelta);
    void Update_Heal(_float fTimeDelta);

private:
    FRAME_LAYER m_FrameLayer = {};
    GAUGE_LAYER m_Layers[ETOI(LAYER_TYPE::END)] = {};
    GAUGE_LAYOUT_DESC m_Layout = {};
    GAUGE_MOTION_DESC m_Motion = {};

    _bool m_bInitializedRatio = { false };

    _float m_fTargetRatio = { 1.f };
    _float m_fFillRatio = { 1.f };

    _bool m_bGhostPlaying = { false };
    _float m_fGhostRatio = { 1.f };
    _float m_fGhostDelayAcc = { 0.f };

    _bool m_bHealPlaying = { false };
    _float m_fHealRatio = { 1.f };

public:
    static CUI_GaugeBarCom* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END
