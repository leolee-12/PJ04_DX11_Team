#include "Mesh.h"
#include "Model.h"
#include "Bone.h"
#include "Shader.h"
#include "Picking_Utils.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer { pDevice, pContext }
{
}

CMesh::CMesh(const CMesh& Prototype)
    : CVIBuffer(Prototype)
{
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, CModel* pOwner, const MESH_DATA& data, _fmatrix PreTransformMatrix, _bool bPickable)
{
	m_strName = data.strName;
	m_bPickable = bPickable;

    HRESULT hr = MODEL::NONANIM == eType ? Ready_NonAnim(data, PreTransformMatrix) : Ready_Anim(pOwner, data);
    if (FAILED(hr))
        return E_FAIL;

	return S_OK;
}

HRESULT CMesh::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CMesh::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones)
{
    ZeroMemory(m_BoneMatrices, sizeof(_float4x4) * g_iNumMeshBones);

    for (size_t i = 0; i < m_iNumBones; i++)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i],
            XMLoadFloat4x4(&m_BoneOffsetMatrices[i]) *
            XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrixPtr()));
    }

    return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

_bool CMesh::Ray_AABB(_fvector vOrigin, _fvector vDir) const
{
    _float3 origin, dir;
    XMStoreFloat3(&origin, vOrigin);
    XMStoreFloat3(&dir, vDir);

    float tMin = -FLT_MAX, tMax = FLT_MAX;

    // X축
    if (fabsf(dir.x) < 1e-6f) {
        if (origin.x < m_vAABBMin.x || origin.x > m_vAABBMax.x) return false;
    }
    else {
        float t1 = (m_vAABBMin.x - origin.x) / dir.x;
        float t2 = (m_vAABBMax.x - origin.x) / dir.x;
        if (t1 > t2) swap(t1, t2);
        tMin = max(tMin, t1);
        tMax = min(tMax, t2);
    }

    // Y축
    if (fabsf(dir.y) < 1e-6f) {
        if (origin.y < m_vAABBMin.y || origin.y > m_vAABBMax.y) return false;
    }
    else {
        float t1 = (m_vAABBMin.y - origin.y) / dir.y;
        float t2 = (m_vAABBMax.y - origin.y) / dir.y;
        if (t1 > t2) swap(t1, t2);
        tMin = max(tMin, t1);
        tMax = min(tMax, t2);
    }

    // Z축
    if (fabsf(dir.z) < 1e-6f) {
        if (origin.z < m_vAABBMin.z || origin.z > m_vAABBMax.z) return false;
    }
    else {
        float t1 = (m_vAABBMin.z - origin.z) / dir.z;
        float t2 = (m_vAABBMax.z - origin.z) / dir.z;
        if (t1 > t2) swap(t1, t2);
        tMin = max(tMin, t1);
        tMax = min(tMax, t2);
    }

    return tMin <= tMax && tMax >= 0.f;
}

_bool CMesh::Pick(_fvector vOrigin, _fvector vDir, _float3* pOutHit, _float* pOutDist) const
{
    if (!m_bPickable) return false;

    return CPicking_Utils::RayToMesh(
        vOrigin, vDir,
        m_PickingPositions.data(), (_uint)m_PickingPositions.size(),
        m_PickingIndices.data(), (_uint)m_PickingIndices.size(),
        pOutHit, pOutDist
    );
}

HRESULT CMesh::Ready_NonAnim(const MESH_DATA& data, _fmatrix PreTransformMatrix)
{
    m_iMaterialIndex = data.iMaterialIndex;
    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXMESH);
    m_iNumVertices = (_uint)data.NonAnimVertices.size();
    m_iNumIndices = (_uint)data.Indices.size();
    m_iIndexStride = 4;
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // Vertex Buffer
    VTXMESH* pVertices = new VTXMESH[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXMESH) * m_iNumVertices);

    for (_uint i = 0; i < m_iNumVertices; ++i)
    {
        const VTXMESH_DATA& src = data.NonAnimVertices[i];

        pVertices[i].vTexcoord = src.vTexcoord;

        XMStoreFloat3(&pVertices[i].vPosition,
            XMVector3TransformCoord(XMLoadFloat3(&src.vPosition), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vNormal,
            XMVector3TransformNormal(XMLoadFloat3(&src.vNormal), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vTangent,
            XMVector3TransformNormal(XMLoadFloat3(&src.vTangent), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vBinormal,
            XMVector3TransformNormal(XMLoadFloat3(&src.vBinormal), PreTransformMatrix));
    }

    for (_uint i = 0; i < m_iNumVertices; ++i)
    {
        m_vAABBMin.x = min(m_vAABBMin.x, pVertices[i].vPosition.x);
        m_vAABBMin.y = min(m_vAABBMin.y, pVertices[i].vPosition.y);
        m_vAABBMin.z = min(m_vAABBMin.z, pVertices[i].vPosition.z);
        m_vAABBMax.x = max(m_vAABBMax.x, pVertices[i].vPosition.x);
        m_vAABBMax.y = max(m_vAABBMax.y, pVertices[i].vPosition.y);
        m_vAABBMax.z = max(m_vAABBMax.z, pVertices[i].vPosition.z);
    }

    if (m_bPickable)
    {
        m_PickingPositions.reserve(m_iNumVertices);
        for (_uint i = 0; i < m_iNumVertices; ++i)
            m_PickingPositions.push_back(pVertices[i].vPosition);

        m_PickingIndices.assign(data.Indices.begin(), data.Indices.end());
    }

    D3D11_BUFFER_DESC VBDesc{};
    VBDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.StructureByteStride = m_iVertexStride;

    D3D11_SUBRESOURCE_DATA VBInitData{};
    VBInitData.pSysMem = pVertices;

    HRESULT hr = m_pDevice->CreateBuffer(&VBDesc, &VBInitData, &m_pVB);
    Safe_Delete_Array(pVertices);
    if (FAILED(hr)) return E_FAIL;

    // Index Buffer
    D3D11_BUFFER_DESC IBDesc{};
    IBDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.StructureByteStride = m_iIndexStride;

    D3D11_SUBRESOURCE_DATA IBInitData{};
    IBInitData.pSysMem = data.Indices.data();

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitData, &m_pIB)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMesh::Ready_Anim(CModel* pOwner, const MESH_DATA& data)
{
    m_iMaterialIndex = data.iMaterialIndex;
    m_iNumVertexBuffers = 1;
    m_iVertexStride = sizeof(VTXANIMMESH);
    m_iNumVertices = (_uint)data.AnimVertices.size();
    m_iNumIndices = (_uint)data.Indices.size();
    m_iIndexStride = 4;
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // Vertex Buffer - 블렌드 가중치는 컨버터에서 이미 산포됨
    VTXANIMMESH* pVertices = new VTXANIMMESH[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

    for (_uint i = 0; i < m_iNumVertices; ++i)
    {
        const VTXANIMMESH_DATA& src = data.AnimVertices[i];
        pVertices[i].vPosition = src.vPosition;
        pVertices[i].vNormal = src.vNormal;
        pVertices[i].vTexcoord = src.vTexcoord;
        pVertices[i].vTangent = src.vTangent;
        pVertices[i].vBinormal = src.vBinormal;
        pVertices[i].vBlendIndex = src.vBlendIndex;
        pVertices[i].vBlendWeight = src.vBlendWeight;
    }

    for (_uint i = 0; i < m_iNumVertices; ++i)
    {
        m_vAABBMin.x = min(m_vAABBMin.x, pVertices[i].vPosition.x);
        m_vAABBMin.y = min(m_vAABBMin.y, pVertices[i].vPosition.y);
        m_vAABBMin.z = min(m_vAABBMin.z, pVertices[i].vPosition.z);
        m_vAABBMax.x = max(m_vAABBMax.x, pVertices[i].vPosition.x);
        m_vAABBMax.y = max(m_vAABBMax.y, pVertices[i].vPosition.y);
        m_vAABBMax.z = max(m_vAABBMax.z, pVertices[i].vPosition.z);
    }

    D3D11_BUFFER_DESC VBDesc{};
    VBDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.StructureByteStride = m_iVertexStride;

    D3D11_SUBRESOURCE_DATA VBInitData{};
    VBInitData.pSysMem = pVertices;

    HRESULT hr = m_pDevice->CreateBuffer(&VBDesc, &VBInitData, &m_pVB);
    Safe_Delete_Array(pVertices);
    if (FAILED(hr)) return E_FAIL;

    // 메시별 본 - 이름으로 인덱스 조회 + 오프셋 행렬만
    m_iNumBones = (_uint)data.Bones.size();
    m_BoneIndices.reserve(m_iNumBones);
    m_BoneOffsetMatrices.reserve(m_iNumBones);

    for (const auto& bone : data.Bones)
    {
        _int iBoneIndex = pOwner->Get_BoneIndex(bone.strName);
        if (-1 == iBoneIndex) return E_FAIL;
        m_BoneIndices.push_back((_uint)iBoneIndex);
        m_BoneOffsetMatrices.push_back(bone.OffsetMatrix);
    }

    // 0-bone 특수 처리
    if (0 == m_iNumBones)
    {
        m_iNumBones = 1;
        _int iBoneIndex = pOwner->Get_BoneIndex(m_strName);
        if (-1 == iBoneIndex) return E_FAIL;
        _float4x4 identity;
        XMStoreFloat4x4(&identity, XMMatrixIdentity());
        m_BoneOffsetMatrices.push_back(identity);
        m_BoneIndices.push_back((_uint)iBoneIndex);
    }

    // Index Buffer
    D3D11_BUFFER_DESC IBDesc{};
    IBDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.StructureByteStride = m_iIndexStride;

    D3D11_SUBRESOURCE_DATA IBInitData{};
    IBInitData.pSysMem = data.Indices.data();

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitData, &m_pIB)))
        return E_FAIL;

    return S_OK;
}

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, CModel* pOwner, const MESH_DATA& data, _fmatrix PreTransformMatrix, _bool bPickable)
{
    CMesh* pInstance = new CMesh(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(eType, pOwner, data, PreTransformMatrix, bPickable)))
    {
        MSG_BOX("Failed to Created : CMesh");
        Safe_Release(pInstance);
    }

    return pInstance;
}


CComponent* CMesh::Clone(void* pArg)
{
    return nullptr;
}

void CMesh::Free()
{
    __super::Free();
}
