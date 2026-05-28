#include "VIBuffer_Trail.h"

CVIBuffer_Trail::CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Trail::CVIBuffer_Trail(const CVIBuffer_Trail& Prototype)
    : CVIBuffer(Prototype)
    , m_iMaxSamples{ Prototype.m_iMaxSamples }
    , m_fSampleLifeTime{ Prototype.m_fSampleLifeTime }
{
    /* 베이스 카피 ctor가 m_pVB를 AddRef했지만,
       이 클래스는 인스턴스별 다이내믹 VB를 새로 만들 거라 떼어낸다. */
    Safe_Release(m_pVB);
    m_pVB = nullptr;
}

HRESULT CVIBuffer_Trail::Initialize_Prototype()
{
    /* 프로토타입에선 stride/포맷만 정의. 실제 VB는 Clone 후 Initialize에서 생성. */
    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXTRAIL);

    m_iNumIndices = 0;
    m_iIndexStride = 0;
    m_eIndexFormat = DXGI_FORMAT_UNKNOWN;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    return S_OK;
}

HRESULT CVIBuffer_Trail::Initialize(void* pArg)
{
    if (pArg != nullptr)
    {
        TRAIL_DESC* pDesc = static_cast<TRAIL_DESC*>(pArg);
        m_iMaxSamples = pDesc->iMaxSamples;
        m_fSampleLifeTime = pDesc->fSampleLifeTime;
    }
    if (m_iMaxSamples < 2) m_iMaxSamples = 2;

    /* 정점 수 = 샘플 쌍 수 × 2 (base/tip 한 쌍) */
    m_iNumVertices = m_iMaxSamples * 2;

    return Create_DynamicVB();
}

HRESULT CVIBuffer_Trail::Create_DynamicVB()
{
    D3D11_BUFFER_DESC BufferDesc{};
    BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;
    BufferDesc.StructureByteStride = m_iVertexStride;

    if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pVB)))
        return E_FAIL;

    return S_OK;
}

void CVIBuffer_Trail::Push_Sample(const _float3& vBase, const _float3& vTip)
{
    TRAIL_SAMPLE sample{ vBase, vTip, 0.f };
    m_Samples.push_back(sample);

    while (m_Samples.size() > m_iMaxSamples)
        m_Samples.pop_front();
}

void CVIBuffer_Trail::Update(_float fTimeDelta)
{
    for (auto& s : m_Samples)
        s.fAge += fTimeDelta;

    while (!m_Samples.empty() && m_Samples.front().fAge > m_fSampleLifeTime)
        m_Samples.pop_front();
}

void CVIBuffer_Trail::Clear()
{
    m_Samples.clear();
}

void CVIBuffer_Trail::Upload_Vertices()
{
    if (m_Samples.size() < 2)
        return;

    D3D11_MAPPED_SUBRESOURCE Mapped{};
    if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
        return;

    VTXTRAIL* pV = static_cast<VTXTRAIL*>(Mapped.pData);
    const _uint N = (_uint)m_Samples.size();
    const _float fInvDenom = 1.f / static_cast<_float>(N - 1);

    for (_uint i = 0; i < N; ++i)
    {
        const TRAIL_SAMPLE& s = m_Samples[i];
        const _float t = static_cast<_float>(i) * fInvDenom;  /* 0=꼬리 → 1=선두 */

        pV[i * 2 + 0].vPosition = s.vBase;
        pV[i * 2 + 0].vTexcoord = _float2(t, 0.f);
        pV[i * 2 + 0].fAge = s.fAge;

        pV[i * 2 + 1].vPosition = s.vTip;
        pV[i * 2 + 1].vTexcoord = _float2(t, 1.f);
        pV[i * 2 + 1].fAge = s.fAge;
    }

    m_pContext->Unmap(m_pVB, 0);
}

HRESULT CVIBuffer_Trail::Bind_Resources()
{
    Upload_Vertices();

    ID3D11Buffer* pVBs[] = { m_pVB };
    _uint         strides[] = { m_iVertexStride };
    _uint         offsets[] = { 0 };

    m_pContext->IASetVertexBuffers(0, 1, pVBs, strides, offsets);
    m_pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);
    return S_OK;
}

HRESULT CVIBuffer_Trail::Render()
{
    if (m_Samples.size() < 2)
        return S_OK;

    const _uint iNumActiveVerts = (_uint)m_Samples.size() * 2;
    m_pContext->Draw(iNumActiveVerts, 0);
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
    m_Samples.clear();
    __super::Free();
}