#include "VIBuffer_Point.h"

CVIBuffer_Point::CVIBuffer_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer{ pDevice, pContext } {
}

CVIBuffer_Point::CVIBuffer_Point(const CVIBuffer_Point& Prototype)
    : CVIBuffer(Prototype) {
}

HRESULT CVIBuffer_Point::Initialize_Prototype()
{
    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXPOS);
    m_iNumVertices = 1;
    m_iNumIndices = 0;
    m_iIndexStride = 0;
    m_eIndexFormat = DXGI_FORMAT_UNKNOWN;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    VTXPOS v{ XMFLOAT3(0.f, 0.f, 0.f) };

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(VTXPOS);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.StructureByteStride = sizeof(VTXPOS);

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = &v;

    return m_pDevice->CreateBuffer(&desc, &init, &m_pVB);
}

HRESULT CVIBuffer_Point::Render()
{
    m_pContext->Draw(1, 0);
    return S_OK;
}

CVIBuffer_Point* CVIBuffer_Point::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto* p = new CVIBuffer_Point(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("CVIBuffer_Point Create"); Safe_Release(p); }
    return p;
}

CComponent* CVIBuffer_Point::Clone(void* pArg)
{
    auto* p = new CVIBuffer_Point(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("CVIBuffer_Point Clone"); Safe_Release(p); }
    return p;
}

void CVIBuffer_Point::Free() { __super::Free(); }