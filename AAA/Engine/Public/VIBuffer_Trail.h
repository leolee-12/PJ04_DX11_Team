#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Trail final : public CVIBuffer
{
public:
    struct TRAIL_SAMPLE
    {
        _float3 vBase{};
        _float3 vTip{};
        _float  fAge{};
        _float  fDistance{};            // 누적 이동 거리, U 좌표에 사용 가능
        _bool   bStartsNewSegment{};    // 이전 샘플과 연결할지 여부
    };

    struct TRAIL_DESC
    {
        _uint  iMaxSamples{ 64 };
        _float fSampleLifeTime{ 0.3f };
        _uint  iSmoothSegments{ 4 };
        _float fSplineSmoothness{ 1.f };
        _bool  bDistanceUV{ true };
        _float fTailWidthScale{ 0.f };
        _float fHeadWidthScale{ 1.f };
    };

private:
    CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CVIBuffer_Trail(const CVIBuffer_Trail& Prototype);
    virtual ~CVIBuffer_Trail() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Bind_Resources() override;
    virtual HRESULT Render() override;

public:
    HRESULT Configure(const TRAIL_DESC& TrailDesc);

    void Push_Sample(const _float3& vBase, const _float3& vTip, _float fInitialAge = 0.f);
    void Begin_NewSegment();
    void Update(_float fTimeDelta);
    void Clear();

    _bool Is_Empty() const { return m_Samples.empty(); }
    _bool Is_Renderable() const;
    _uint Get_NumSamples() const { return static_cast<_uint>(m_Samples.size()); }
    _float Get_LifeTime() const { return m_Desc.fSampleLifeTime; }
    const TRAIL_DESC& Get_Description() const { return m_Desc; }

private:
    TRAIL_DESC m_Desc{};
    deque<TRAIL_SAMPLE> m_Samples;
    // 렌더용 보간 렌더링용 샘플
    vector<TRAIL_SAMPLE> m_RenderSamples;
    // GPU에 올릴 최종 정점 배열
    vector<VTXTRAIL> m_RenderVertices;
    _uint m_iNumActiveVertices{};
    _bool m_bBreakBeforeNextSample{ true };

private:
    HRESULT Create_DynamicVB();
    HRESULT Upload_Vertices();
    void Build_RenderVertices();

public:
    static CVIBuffer_Trail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END
