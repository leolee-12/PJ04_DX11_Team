#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

class CUI_GaugeFill;
class CUI_Image;

typedef struct tagUIBounceDesc
{
    _float2 vDirection = { 0.f, 1.f };
    _float  fDistance = { 0.f };
    _float  fDuration = { 0.f };
    _float  fWaveCount = { 1.f };
    _float  fDamping = { 0.f };
    _bool   bRestoreOnFinish = { true };
} UI_BOUNCE_DESC;

typedef struct tagUIFadeDesc
{
    _float  fFromAlpha = { -1.f };
    _float  fToAlpha = { 1.f };
    _float  fDuration = { 0.f };
    _float  fDelay = { 0.f };
    _bool   bRestoreOnFinish = { false };
} UI_FADE_DESC;

typedef struct tagUIGaugeSegments
{
    CUI_GaugeFill* pLeft = { nullptr };
    CUI_GaugeFill* pCenter = { nullptr };
    CUI_GaugeFill* pRight = { nullptr };
} UI_GAUGE_SEGMENTS;

typedef struct tagUIFrameSegments
{
    CUI_Image* pLeft = { nullptr };
    CUI_GaugeFill* pCenter = { nullptr };
    CUI_Image* pRight = { nullptr };
} UI_FRAME_SEGMENTS;

typedef struct tagUIGaugeLayoutDesc
{
    _float fLeftWidth = { 18.f };
    _float fCenterWidth = { 130.f };
    _float fRightWidth = { 18.f };
    _float fHeight = { 18.f };

    _float fFrameZ = { 1.4f };
    _float fBackZ = { 1.3f };
    _float fHealZ = { 1.1f };
    _float fFillZ = { 1.0f };
} UI_GAUGE_LAYOUT_DESC;

typedef struct tagUIGaugeMotionDesc
{
    _float fHealSpeed = { 2.8f };
} UI_GAUGE_MOTION_DESC;

typedef struct tagUIGaugeLayer
{
    UI_GAUGE_SEGMENTS Segments = {};
    _float4 vColor = { 1.f, 1.f, 1.f, 1.f };
    _bool bBound = { false };
} UI_GAUGE_LAYER;

typedef struct tagUIGaugeSegmentRatio
{
    _float fLeft = { 1.f };
    _float fCenter = { 1.f };
    _float fRight = { 1.f };
} UI_GAUGE_SEGMENT_RATIO;

typedef struct tagUIFrameLayer
{
    UI_FRAME_SEGMENTS Segments = {};
    _bool bBound = { false };
} UI_FRAME_LAYER;

NS_END
