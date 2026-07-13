#pragma once

#include "Effect_NonParticle.h"

NS_BEGIN(Engine)

class CVIBuffer_Trail;

class ENGINE_DLL CEffect_Trail abstract : public CEffect_NonParticle
{
    GENERATED_BODY_ABSTRACT(CEffect_Trail)

    PROPERTY(_bool, m_bAutoSample,          L"Auto Sample",         L"Trail - Sampling");
    PROPERTY(_float3, m_vBaseOffset,        L"Base Offset",         L"Trail - Sampling");
    PROPERTY(_float3, m_vTipOffset,         L"Tip Offset",          L"Trail - Sampling");
    PROPERTY(_uint, m_iMaxSamples,          L"Max Samples",         L"Trail - Sampling");
    PROPERTY(_float, m_fSampleLifeTime,     L"Sample Life Time",    L"Trail - Sampling");
    PROPERTY(_float, m_fSampleInterval,     L"Sample Interval",     L"Trail - Sampling");
    PROPERTY(_float, m_fMinSampleDistance,  L"Min Sample Distance", L"Trail - Sampling");
    PROPERTY(_float, m_fTeleportDistance,   L"Teleport Distance",   L"Trail - Sampling");

    PROPERTY(_uint, m_iSmoothSegments,      L"Smooth Segments",     L"Trail - Shape");
    PROPERTY(_float, m_fSplineSmoothness,   L"Spline Smoothness",   L"Trail - Shape");
    PROPERTY(_float, m_fTailWidthScale,     L"Tail Width Scale",    L"Trail - Shape");
    PROPERTY(_float, m_fHeadWidthScale,     L"Head Width Scale",    L"Trail - Shape");

    PROPERTY(_bool, m_bDistanceUV,          L"Distance UV",         L"Trail - Rendering");
    PROPERTY(_float, m_fEdgeSoftness,       L"Edge Softness",       L"Trail - Rendering");
    PROPERTY(_float, m_fHeadFadeRatio,      L"Head Fade Ratio",     L"Trail - Rendering");
    PROPERTY(_float, m_fTailFadeRatio,      L"Tail Fade Ratio",     L"Trail - Rendering");

public:
    struct EFFECT_TRAIL_DESC : public CEffect_NonParticle::EFFECT_NONEPARTICLE_DESC
    {
        _uint iVIBufferLevel{};
        _wstring wstrVIBufferTag;

        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

protected:
    CEffect_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Trail(const CEffect_Trail& Prototype);
    virtual ~CEffect_Trail() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Effect_Start() override;

public:
    void Start_Emission();
    void Stop_Emission();
    void Clear_Trail();
    void Begin_NewSegment();

    void Set_SourceMatrix(const _float4x4* pSourceMatrix);
    void Push_WorldSample(const _float3& vBase, const _float3& vTip);

    _bool Is_Emitting() const { return m_bEmitting; }
    _bool Is_TrailRenderable() const;
    _bool Is_TrailFinished() const;

protected:
    virtual void On_Deserialized() override;

private:
    HRESULT Ready_Components();
    HRESULT Configure_Buffer();
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShaderValue();

    void Sample_Automatically(_float fTimeDelta);
    void Push_FilteredSample(const _float3& vBase, const _float3& vTip, _float fInitialAge = 0.f);
    void Reset_SamplingState();
    void Compute_WorldEndpoints(_float3& vBase, _float3& vTip) const;

private:
    CVIBuffer_Trail* m_pVIBuffer{};

    _uint m_iVIBufferLevel{};
    _wstring m_wstrVIBufferTag;
    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;

    const _float4x4* m_pSourceMatrix{};
    _bool m_bEmitting{ true };
    _bool m_bHasPreviousFrame{};
    _bool m_bHasLastEmitted{};
    _float m_fSampleAccumulator{};
    _float3 m_vPreviousFrameBase{};
    _float3 m_vPreviousFrameTip{};
    _float3 m_vLastEmittedBase{};
    _float3 m_vLastEmittedTip{};

private:
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END
