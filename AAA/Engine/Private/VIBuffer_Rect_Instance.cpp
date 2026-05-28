#include "VIBuffer_Rect_Instance.h"
#include "GameInstance.h"

CVIBuffer_Rect_Instance::CVIBuffer_Rect_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance(pDevice, pContext)
{

}

CVIBuffer_Rect_Instance::CVIBuffer_Rect_Instance(const CVIBuffer_Rect_Instance& Prototype)
	: CVIBuffer_Instance(Prototype)
    , m_pSpeeds { Prototype.m_pSpeeds }
    , m_isLoop { Prototype.m_isLoop }
    
{
	
}

HRESULT CVIBuffer_Rect_Instance::Initialize_Prototype(void* pInitialDesc)
{
    m_iNumVertexBuffers = 2;
    m_iNumVertices = 4;
    m_iVertexStride = sizeof(VTXTEX);

    m_iNumIndices = 6;
    m_iIndexStride = 2;
    m_eIndexFormat = DXGI_FORMAT_R16_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    auto        pDesc = static_cast<CVIBuffer_Rect_Instance::RECT_INSTANCE_DESC*>(pInitialDesc);

    m_iInstanceStride = sizeof(VTXPARTICLE_INSTANCE);
    m_iNumInstances = pDesc->iNumInstance;
    m_iIndexCountPerInstance = m_iNumIndices;


    /* 네모를 표현해주기 위한 정점, 인덱스를 생성한다. */
    D3D11_BUFFER_DESC           VertexBufferDesc{};
    VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
    VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VertexBufferDesc.CPUAccessFlags = 0;
    VertexBufferDesc.MiscFlags = 0;
    VertexBufferDesc.StructureByteStride = m_iVertexStride;

    VTXTEX* pVertices = new VTXTEX[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXTEX) * m_iNumVertices);

    pVertices[0].vPosition = _float3(-0.5f, 0.5f, 0.f);
    pVertices[0].vTexcoord = _float2(0.f, 0.f);

    pVertices[1].vPosition = _float3(0.5f, 0.5f, 0.f);
    pVertices[1].vTexcoord = _float2(1.f, 0.f);

    pVertices[2].vPosition = _float3(0.5f, -0.5f, 0.f);
    pVertices[2].vTexcoord = _float2(1.f, 1.f);

    pVertices[3].vPosition = _float3(-0.5f, -0.5f, 0.f);
    pVertices[3].vTexcoord = _float2(0.f, 1.f);

    D3D11_SUBRESOURCE_DATA      VertexInitialData{};
    VertexInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);



    D3D11_BUFFER_DESC           IndexBufferDesc{};
    IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
    IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.CPUAccessFlags = 0;
    IndexBufferDesc.MiscFlags = 0;
    IndexBufferDesc.StructureByteStride = m_iIndexStride;


    _ushort* pIndices = new _ushort[m_iNumIndices];
    ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

    pIndices[0] = 0;
    pIndices[1] = 1;
    pIndices[2] = 2;

    pIndices[3] = 0;
    pIndices[4] = 2;
    pIndices[5] = 3;


    D3D11_SUBRESOURCE_DATA      IndexInitialData{};
    IndexInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);



    m_InstanceBufferDesc.ByteWidth = m_iNumInstances * m_iInstanceStride;
    m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_InstanceBufferDesc.MiscFlags = 0;
    m_InstanceBufferDesc.StructureByteStride = m_iInstanceStride;

    m_pInstanceVertices = new VTXPARTICLE_INSTANCE[m_iNumInstances];
    ZeroMemory(m_pInstanceVertices, sizeof(VTXPARTICLE_INSTANCE) * m_iNumInstances);

    m_pSpeeds = new _float[m_iNumInstances];

    m_isLoop = pDesc->isLoop;

    for (size_t i = 0; i < m_iNumInstances; i++)
    {
        _float      fSize = m_pGameInstance_Proxy->RandomFloat(pDesc->vSize.x, pDesc->vSize.y);

        m_pSpeeds[i] = m_pGameInstance_Proxy->RandomFloat(pDesc->vSpeed.x, pDesc->vSpeed.y);

        m_pInstanceVertices[i].vRight = _float4(fSize, 0.f, 0.f, 0.f);
        m_pInstanceVertices[i].vUp = _float4(0.f, fSize, 0.f, 0.f);
        m_pInstanceVertices[i].vLook = _float4(0.f, 0.f, fSize, 0.f);
        m_pInstanceVertices[i].vTranslation = _float4(
            m_pGameInstance_Proxy->RandomFloat(pDesc->vCenter.x - pDesc->vRange.x * 0.5f, pDesc->vCenter.x + pDesc->vRange.x * 0.5f),
            m_pGameInstance_Proxy->RandomFloat(pDesc->vCenter.y - pDesc->vRange.y * 0.5f, pDesc->vCenter.y + pDesc->vRange.y * 0.5f),
            m_pGameInstance_Proxy->RandomFloat(pDesc->vCenter.z - pDesc->vRange.z * 0.5f, pDesc->vCenter.z + pDesc->vRange.z * 0.5f),
            1.f
        );

        m_pInstanceVertices[i].vLifeTime = _float2(
            m_pGameInstance_Proxy->RandomFloat(pDesc->vLifeTime.x, pDesc->vLifeTime.y),
            0.f
        );
    }

  
   

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Initialize(void* pArg)
{
    D3D11_SUBRESOURCE_DATA      InstanceInitialData{};
    InstanceInitialData.pSysMem = m_pInstanceVertices;

    if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &InstanceInitialData, &m_pVBInstance)))
        return E_FAIL;

	return S_OK;
}

void CVIBuffer_Rect_Instance::Drop(_float fTimeDelta)
{
    D3D11_MAPPED_SUBRESOURCE        SubResource{};

    m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

    VTXPARTICLE_INSTANCE*       pInstanceVertices = static_cast<VTXPARTICLE_INSTANCE*>(SubResource.pData);

    for (size_t i = 0; i < m_iNumInstances; i++)
    {
        pInstanceVertices[i].vTranslation.y -= m_pSpeeds[i] * fTimeDelta;
        pInstanceVertices[i].vLifeTime.y += fTimeDelta;

        if (true == m_isLoop && 
            pInstanceVertices[i].vLifeTime.x < pInstanceVertices[i].vLifeTime.y)
        {
            pInstanceVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
            pInstanceVertices[i].vLifeTime.y = 0.f;
        }
    }   

    m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Rect_Instance::Spread(_float fTimeDelta)
{
}

CVIBuffer_Rect_Instance* CVIBuffer_Rect_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pInitialDesc)
{
    CVIBuffer_Rect_Instance* pInstance = new CVIBuffer_Rect_Instance(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pInitialDesc)))
    {
        MSG_BOX("Failed to Created : CVIBuffer_Rect_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Rect_Instance::Clone(void* pArg)
{
    CVIBuffer_Rect_Instance* pInstance = new CVIBuffer_Rect_Instance(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CVIBuffer_Rect_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CVIBuffer_Rect_Instance::Free()
{
	__super::Free();

    
    if (false == m_isCloned)
        Safe_Delete_Array(m_pSpeeds);
}
