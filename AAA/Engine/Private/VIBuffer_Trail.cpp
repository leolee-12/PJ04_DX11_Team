#include "VIBuffer_Trail.h"

namespace
{
    _float Distance(const _float3& vA, const _float3& vB)
    {
        return XMVectorGetX(XMVector3Length(XMLoadFloat3(&vB) - XMLoadFloat3(&vA)));
    }

    _float3 Extrapolate(const _float3& vFrom, const _float3& vToward)
    {
        _float3 vResult{};
        XMStoreFloat3(&vResult, XMLoadFloat3(&vFrom) * 2.f - XMLoadFloat3(&vToward));
        return vResult;
    }

    _float Knot(_float fCurrent, const _float3& vA, const _float3& vB)
    {
        const _float fChord = (std::max)(Distance(vA, vB), 0.0001f);
        return fCurrent + sqrtf(fChord);
    }

    _vector InterpolateAt(_vector vA, _vector vB, _float fTimeA, _float fTimeB, _float fTime)
    {
        const _float fRange = fTimeB - fTimeA;
        if (fabsf(fRange) <= Helper::fEpsilon)
            return vB;

        return vA * ((fTimeB - fTime) / fRange) + vB * ((fTime - fTimeA) / fRange);
    }

    _float3 CentripetalCatmullRom(const _float3& vPoint0Position, const _float3& vPoint1Position,
        const _float3& vPoint2Position, const _float3& vPoint3Position, _float fRatio, _float fSmoothness)
    {
        const _float fTime0 = 0.f;
        const _float fTime1 = Knot(fTime0, vPoint0Position, vPoint1Position);
        const _float fTime2 = Knot(fTime1, vPoint1Position, vPoint2Position);
        const _float fTime3 = Knot(fTime2, vPoint2Position, vPoint3Position);
        const _float fTime = fTime1 + (fTime2 - fTime1) * fRatio;

        const _vector vPoint0 = XMLoadFloat3(&vPoint0Position);
        const _vector vPoint1 = XMLoadFloat3(&vPoint1Position);
        const _vector vPoint2 = XMLoadFloat3(&vPoint2Position);
        const _vector vPoint3 = XMLoadFloat3(&vPoint3Position);

        const _vector vA1 = InterpolateAt(vPoint0, vPoint1, fTime0, fTime1, fTime);
        const _vector vA2 = InterpolateAt(vPoint1, vPoint2, fTime1, fTime2, fTime);
        const _vector vA3 = InterpolateAt(vPoint2, vPoint3, fTime2, fTime3, fTime);
        const _vector vB1 = InterpolateAt(vA1, vA2, fTime0, fTime2, fTime);
        const _vector vB2 = InterpolateAt(vA2, vA3, fTime1, fTime3, fTime);
        const _vector vSpline = InterpolateAt(vB1, vB2, fTime1, fTime2, fTime);
        const _vector vLinear = XMVectorLerp(vPoint1, vPoint2, fRatio);

        _float3 vResult{};
        XMStoreFloat3(&vResult, XMVectorLerp(vLinear, vSpline, fSmoothness));
        return vResult;
    }
}

CVIBuffer_Trail::CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Trail::CVIBuffer_Trail(const CVIBuffer_Trail& Prototype)
    : CVIBuffer(Prototype)
    , m_Desc{ Prototype.m_Desc }
{
    Safe_Release(m_pVB);
    m_pVB = nullptr;
}

HRESULT CVIBuffer_Trail::Initialize_Prototype()
{
    // 정점
    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXTRAIL);
    m_iNumIndices = 0;
    m_iIndexStride = 0;
    m_eIndexFormat = DXGI_FORMAT_UNKNOWN;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    // 인덱스 x

    return S_OK;
}

HRESULT CVIBuffer_Trail::Initialize(void* pArg)
{
    TRAIL_DESC TrailDesc{};

    // 구조체 없으면 기본값
    if (pArg != nullptr)
        TrailDesc = *static_cast<TRAIL_DESC*>(pArg);

    // 설정값 보정
    return Configure(TrailDesc);
}

HRESULT CVIBuffer_Trail::Configure(const TRAIL_DESC& TrailDesc)
{
    TRAIL_DESC ValidatedDesc = TrailDesc;
    ValidatedDesc.iMaxSamples = (std::max)(static_cast<_uint>(2), (std::min)(ValidatedDesc.iMaxSamples, 512u));
    ValidatedDesc.iSmoothSegments = (std::max)(static_cast<_uint>(1), (std::min)(ValidatedDesc.iSmoothSegments, 16u));
    ValidatedDesc.fSampleLifeTime = (std::max)(ValidatedDesc.fSampleLifeTime, Helper::fEpsilon);
    Helper::FloatClamp(ValidatedDesc.fSplineSmoothness, 0.f, 1.f);
    ValidatedDesc.fTailWidthScale = (std::max)(ValidatedDesc.fTailWidthScale, 0.f);
    ValidatedDesc.fHeadWidthScale = (std::max)(ValidatedDesc.fHeadWidthScale, 0.f);

    const _uint iMaxRenderSamples = (ValidatedDesc.iMaxSamples - 1) * ValidatedDesc.iSmoothSegments + 1;
    // Each segment transition needs two degenerate vertices to break the strip.
    const _uint iRequiredVertices = iMaxRenderSamples * 2 + ValidatedDesc.iMaxSamples;
    const _bool bRecreateBuffer = m_pVB == nullptr || m_iNumVertices != iRequiredVertices;

    // 새 버퍼 생성에 실패하면 기존 버퍼와 설정을 그대로 유지한다.
    if (bRecreateBuffer == true)
    {
        const HRESULT hr = Create_DynamicVB(iRequiredVertices);
        if (FAILED(hr))
            return hr;
    }

    m_Desc = ValidatedDesc;
    m_iNumVertices = iRequiredVertices;
    m_RenderSamples.clear();
    m_RenderSamples.reserve(iMaxRenderSamples);
    m_RenderVertices.clear();
    m_RenderVertices.resize(iRequiredVertices);
    Clear();

    return S_OK;
}

HRESULT CVIBuffer_Trail::Create_DynamicVB(_uint iNumVertices)
{
    D3D11_BUFFER_DESC BufferDesc{};
    BufferDesc.ByteWidth = m_iVertexStride * iNumVertices;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* pNewVertexBuffer = nullptr;
    const HRESULT hr = m_pDevice->CreateBuffer(&BufferDesc, nullptr, &pNewVertexBuffer);
    if (FAILED(hr))
    {
        Safe_Release(pNewVertexBuffer);
        return hr;
    }

    Safe_Release(m_pVB);
    m_pVB = pNewVertexBuffer;

    return S_OK;
}

void CVIBuffer_Trail::Push_Sample(const _float3& vBase, const _float3& vTip, _float fInitialAge)
{
    // bStartsNewSegment = 샘플이 비었음 || 다음 샘플을 끊기로 했음
    const _bool bStartsNewSegment = m_Samples.empty() || m_bBreakBeforeNextSample;
    _float fDistance = 0.f;
    if (bStartsNewSegment == false)
    {
        const TRAIL_SAMPLE& PreviousSample = m_Samples.back();
        const _float fMovement = (std::max)(Distance(PreviousSample.vBase, vBase), Distance(PreviousSample.vTip, vTip));
        // fDistance = bStartsNewSegment ? 0 : 이전 거리 + 이동 거리
        fDistance = PreviousSample.fDistance + fMovement;
    }

    m_Samples.push_back({ vBase, vTip, (std::max)(fInitialAge, 0.f), fDistance, bStartsNewSegment });
    m_bBreakBeforeNextSample = false;
    while (m_Samples.size() > m_Desc.iMaxSamples)
        m_Samples.pop_front();
}

void CVIBuffer_Trail::Begin_NewSegment()
{
    m_bBreakBeforeNextSample = true;
}

void CVIBuffer_Trail::Update(_float fTimeDelta)
{
    const _float fDelta = (std::max)(fTimeDelta, 0.f);
    for (TRAIL_SAMPLE& Sample : m_Samples)
        Sample.fAge += fDelta;

    while (m_Samples.empty() == false && m_Samples.front().fAge > m_Desc.fSampleLifeTime)
        m_Samples.pop_front();
}

void CVIBuffer_Trail::Clear()
{
    m_Samples.clear();
    m_RenderSamples.clear();
    m_iNumActiveVertices = 0;
    m_bBreakBeforeNextSample = true;
}

void CVIBuffer_Trail::Build_RenderVertices()
{
    // Sample 보간

    m_RenderSamples.clear();
    m_iNumActiveVertices = 0;

    if (m_Samples.size() < 2)
        return;

    const _uint iSubdivisions = m_Desc.iSmoothSegments;
    const _uint iSampleCount = static_cast<_uint>(m_Samples.size());
    _uint iVertexCursor = 0;
    _bool bHasRenderedSegment = false;

    _uint iSegmentBegin = 0;
    while (iSegmentBegin < iSampleCount)
    {
        // bStartsNewSegment를 만나면 이전 Ribbon과 나누고, Sample이 2개 미만인 Segment는 건너뛴다.
        _uint iSegmentEnd = iSegmentBegin + 1;
        while (iSegmentEnd < iSampleCount && m_Samples[iSegmentEnd].bStartsNewSegment == false)
            ++iSegmentEnd;

        const _uint iSegmentCount = iSegmentEnd - iSegmentBegin;
        if (iSegmentCount < 2)
        {
            iSegmentBegin = iSegmentEnd;
            continue;
        }

        // 보간
        m_RenderSamples.clear();
        for (_uint iLocalIndex = 0; iLocalIndex + 1 < iSegmentCount; ++iLocalIndex)
        {
            const _uint iSampleIndex = iSegmentBegin + iLocalIndex;
            const TRAIL_SAMPLE& FirstSample = m_Samples[iSampleIndex];
            const TRAIL_SAMPLE& SecondSample = m_Samples[iSampleIndex + 1];

            const _float3 vBase0 = iLocalIndex > 0 ? m_Samples[iSampleIndex - 1].vBase : Extrapolate(FirstSample.vBase, SecondSample.vBase);
            const _float3 vTip0 = iLocalIndex > 0 ? m_Samples[iSampleIndex - 1].vTip : Extrapolate(FirstSample.vTip, SecondSample.vTip);
            const _float3 vBase3 = iLocalIndex + 2 < iSegmentCount ? m_Samples[iSampleIndex + 2].vBase : Extrapolate(SecondSample.vBase, FirstSample.vBase);
            const _float3 vTip3 = iLocalIndex + 2 < iSegmentCount ? m_Samples[iSampleIndex + 2].vTip : Extrapolate(SecondSample.vTip, FirstSample.vTip);

            for (_uint iStep = 0; iStep < iSubdivisions; ++iStep)
            {
                const _float fRatio = static_cast<_float>(iStep) / static_cast<_float>(iSubdivisions);

                TRAIL_SAMPLE RenderSample{};
                RenderSample.vBase = CentripetalCatmullRom(vBase0, FirstSample.vBase, SecondSample.vBase, vBase3, fRatio, m_Desc.fSplineSmoothness);
                RenderSample.vTip = CentripetalCatmullRom(vTip0, FirstSample.vTip, SecondSample.vTip, vTip3, fRatio, m_Desc.fSplineSmoothness);
                RenderSample.fAge = FirstSample.fAge + (SecondSample.fAge - FirstSample.fAge) * fRatio;
                RenderSample.fDistance = FirstSample.fDistance + (SecondSample.fDistance - FirstSample.fDistance) * fRatio;
                m_RenderSamples.push_back(RenderSample);
            }
        }

        m_RenderSamples.push_back(m_Samples[iSegmentEnd - 1]);

        const _uint iRenderCount = static_cast<_uint>(m_RenderSamples.size());
        const _float fInverseCount = 1.f / static_cast<_float>(iRenderCount - 1);

        const _uint iBridgeBegin = iVertexCursor;
        if (bHasRenderedSegment == true)
            iVertexCursor += 2;
        const _uint iSegmentVertexBegin = iVertexCursor;

        // 꼬리와 머리의 폭을 조절
        for (_uint iRenderIndex = 0; iRenderIndex < iRenderCount; ++iRenderIndex)
        {
            TRAIL_SAMPLE Sample = m_RenderSamples[iRenderIndex];
            const _float fTrailRatio = static_cast<_float>(iRenderIndex) * fInverseCount;
            const _float fWidthScale = m_Desc.fTailWidthScale +
                (m_Desc.fHeadWidthScale - m_Desc.fTailWidthScale) *
                Helper::FloatSmoothStep(0.f, 1.f, fTrailRatio);

            const _vector vBase = XMLoadFloat3(&Sample.vBase);
            const _vector vTip = XMLoadFloat3(&Sample.vTip);
            const _vector vCenter = (vBase + vTip) * 0.5f;
            const _vector vHalfWidth = (vTip - vBase) * 0.5f * fWidthScale;
            XMStoreFloat3(&Sample.vBase, vCenter - vHalfWidth);
            XMStoreFloat3(&Sample.vTip, vCenter + vHalfWidth);

            const _float fU = m_Desc.bDistanceUV == true ? Sample.fDistance : fTrailRatio;

            // 정점 기록
            VTXTRAIL& BaseVertex = m_RenderVertices[iVertexCursor++];
            BaseVertex.vPosition = Sample.vBase;
            BaseVertex.vTexcoord = { fU, 0.f };
            BaseVertex.fAge = Sample.fAge;

            VTXTRAIL& TipVertex = m_RenderVertices[iVertexCursor++];
            TipVertex.vPosition = Sample.vTip;
            TipVertex.vTexcoord = { fU, 1.f };
            TipVertex.fAge = Sample.fAge;
        }

        if (bHasRenderedSegment == true)
        {
            m_RenderVertices[iBridgeBegin] = m_RenderVertices[iBridgeBegin - 1];
            m_RenderVertices[iBridgeBegin + 1] = m_RenderVertices[iSegmentVertexBegin];
        }

        bHasRenderedSegment = true;
        iSegmentBegin = iSegmentEnd;
    }

    m_iNumActiveVertices = iVertexCursor;
}

_bool CVIBuffer_Trail::Is_Renderable() const
{
    // Sample(base, tip) 하나로 면 그릴 수 없음 최소 2개
    _uint iSamplesInSegment = 0;
    for (const TRAIL_SAMPLE& Sample : m_Samples)
    {
        if (Sample.bStartsNewSegment == true)
            iSamplesInSegment = 0;
        if (++iSamplesInSegment >= 2)
            return true;
    }

    return false;
}

HRESULT CVIBuffer_Trail::Upload_Vertices()
{
    if (m_pVB == nullptr)
        return E_FAIL;

    // 최종 정점을 Dynamic Vertex Buffer에 복사
    Build_RenderVertices();
    if (m_iNumActiveVertices < 4)
        return S_OK;

    D3D11_MAPPED_SUBRESOURCE Mapped{};
    if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
        return E_FAIL;

    memcpy(Mapped.pData, m_RenderVertices.data(), sizeof(VTXTRAIL) * m_iNumActiveVertices);
    m_pContext->Unmap(m_pVB, 0);

    return S_OK;
}

HRESULT CVIBuffer_Trail::Bind_Resources()
{
    // 바인딩
    if (FAILED(Upload_Vertices()))
        return E_FAIL;

    ID3D11Buffer* pVBs[] = { m_pVB };
    _uint strides[] = { m_iVertexStride };
    _uint offsets[] = { 0 };

    m_pContext->IASetVertexBuffers(0, 1, pVBs, strides, offsets);
    m_pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

    return S_OK;
}

HRESULT CVIBuffer_Trail::Render()
{
    if (m_iNumActiveVertices >= 4)
        m_pContext->Draw(m_iNumActiveVertices, 0);

    return S_OK;
}

CVIBuffer_Trail* CVIBuffer_Trail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVIBuffer_Trail* pInstance = new CVIBuffer_Trail(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CVIBuffer_Trail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Trail::Clone(void* pArg)
{
    CVIBuffer_Trail* pInstance = new CVIBuffer_Trail(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CVIBuffer_Trail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CVIBuffer_Trail::Free()
{
    __super::Free();

    Clear();
    m_RenderVertices.clear();
}
