#include "Edit_Grid.h"
#include "Shader.h"

const D3D11_INPUT_ELEMENT_DESC CEdit_Grid::s_Elements[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "COLOR",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

CEdit_Grid::CEdit_Grid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CEdit_Grid::Render(const _float4x4* pView, const _float4x4* pProj)
{
	if (!m_pShader || !m_pVB) return;

	// 월드 행렬은 항등행렬 (그리드는 월드 원점에 고정)
	_float4x4 matWorld;
	XMStoreFloat4x4(&matWorld, XMMatrixIdentity());

	m_pShader->Bind_Matrix("g_WorldMatrix", &matWorld);
	m_pShader->Bind_Matrix("g_ViewMatrix", pView);
	m_pShader->Bind_Matrix("g_ProjMatrix", pProj);
	m_pShader->Begin(0);

	UINT stride = sizeof(VTXGRID), offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_pContext->Draw(m_iVertexCount, 0);
}

HRESULT CEdit_Grid::Initialize(_uint iCellCont, _float fCellSize)
{
	m_pShader = Engine::CShader::Create(
		m_pDevice, m_pContext,
		L"../Bin/ShaderFiles/Shader_Grid.hlsl",
		s_Elements, 2);

	if (m_pShader == nullptr) return E_FAIL;

	Build_Vertices(iCellCont, fCellSize);

	return S_OK;
}

void CEdit_Grid::Build_Vertices(_uint iCellCount, _float fCellSize)
{
	_uint iLineCount = (iCellCount + 1) * 2;
	m_iVertexCount = iLineCount * 2;

	vector<VTXGRID> vertices;
	vertices.reserve(m_iVertexCount);

	_float fHalf = (iCellCount * fCellSize) * 0.5f;

	for (_uint i = 0; i <= iCellCount; ++i)
	{
		_float x = -fHalf + i * fCellSize;

		_float4 vColor = (fabsf(x) < 1e-4f)
			? _float4(1.f, 0.f, 0.f, 1.f)
			: _float4(1.f, 1.f, 1.f, 0.5f);

		vertices.push_back({ _float3(x, 0.f, -fHalf), vColor });
		vertices.push_back({ _float3(x, 0.f, fHalf), vColor });
	}

	for (_uint i = 0; i <= iCellCount; ++i)
	{
		_float z = -fHalf + i * fCellSize;

		_float4 vColor = (fabsf(z) < 1e-4f)
			? _float4(0.f, 0.f, 1.f, 1.f)
			: _float4(1.f, 1.f, 1.f, 0.5f);

		vertices.push_back({ _float3(-fHalf, 0.f, z), vColor });
		vertices.push_back({ _float3(fHalf, 0.f, z), vColor });
	}

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(VTXGRID) * m_iVertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices.data();

	if (FAILED(m_pDevice->CreateBuffer(&bd, &sd, &m_pVB)))
	{
		MSG_BOX("Editor Grid VI Create Fail");
	}
}

CEdit_Grid* CEdit_Grid::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iCellCount, _float fCellSize)
{
	CEdit_Grid* pInstance = new CEdit_Grid(pDevice, pContext);

	if (pInstance == nullptr) return nullptr;

	if (FAILED(pInstance->Initialize(iCellCount, fCellSize)))
	{
		MSG_BOX("Failed to Created : CEdit_Grid");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_Grid::Free()
{
	__super::Free();

	Safe_Release(m_pVB);
	Safe_Release(m_pShader);
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
