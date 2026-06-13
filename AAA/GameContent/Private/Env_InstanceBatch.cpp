#include "Env_InstanceBatch.h"
#include "GameContent_const.h"
#include "EnvObject_Static.h"

#include "GameInstance.h"

namespace
{
	static constexpr _uint INSTANCE_MIN_COUNT = 2;
	static constexpr _uint INSTANCE_MIN_SAVED_DRAWS = 2;
}

CEnv_InstanceBatch::CEnv_InstanceBatch(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CEnv_InstanceBatch::CEnv_InstanceBatch(const CEnv_InstanceBatch& Prototype)
	: CGameObject(Prototype)
{
}

void CEnv_InstanceBatch::Submit(CEnvObject_Static* pObj, _uint64 iCurrentFrame)
{
	if (nullptr == pObj)
		return;

	BeginFrame_IfNeeded(iCurrentFrame);

	INSTANCE_SUBMIT_DATA tData{};
	tData.pObj = pObj;
	Safe_AddRef(pObj);

	const _float4x4* pWorld = pObj->Get_Transform()->Get_WorldMatrixPtr();
	tData.InstanceData.vRight		= _float4(pWorld->_11, pWorld->_12, pWorld->_13, pWorld->_14);
	tData.InstanceData.vUp			= _float4(pWorld->_21, pWorld->_22, pWorld->_23, pWorld->_24);
	tData.InstanceData.vLook		= _float4(pWorld->_31, pWorld->_32, pWorld->_33, pWorld->_34);
	tData.InstanceData.vPosition	= _float4(pWorld->_41, pWorld->_42, pWorld->_43, pWorld->_44);

	m_Submitted.push_back(tData);
}

HRESULT CEnv_InstanceBatch::Ensure_InstanceBuffer(_uint iRequiredCount)
{
	if (iRequiredCount <= m_iInstanceCapacity && nullptr != m_pInstanceBuffer)
		return S_OK;

	Safe_Release(m_pInstanceBuffer);

	m_iInstanceCapacity = max(iRequiredCount, m_iInstanceCapacity * 2);
	if (0 == m_iInstanceCapacity)
		m_iInstanceCapacity = 1;

	D3D11_BUFFER_DESC tDesc{};
	tDesc.ByteWidth = sizeof(ENV_INSTANCE_DATA) * m_iInstanceCapacity;
	tDesc.Usage = D3D11_USAGE_DYNAMIC;
	tDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tDesc.StructureByteStride = sizeof(ENV_INSTANCE_DATA);

	if (FAILED(m_pDevice->CreateBuffer(&tDesc, nullptr, &m_pInstanceBuffer)))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Update_InstanceBuffer()
{
	const _uint iCount = static_cast<_uint>(m_Submitted.size());
	if (0 == iCount)
		return S_OK;

	if (FAILED(Ensure_InstanceBuffer(iCount)))
		return E_FAIL;

	D3D11_MAPPED_SUBRESOURCE tMapped{};
	if(FAILED(m_pContext->Map(m_pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &tMapped)))
		return E_FAIL;

	vector<ENV_INSTANCE_DATA> InstanceTable;
	InstanceTable.reserve(iCount);

	for (const auto& item : m_Submitted)
		InstanceTable.push_back(item.InstanceData);

	memcpy(tMapped.pData, InstanceTable.data(), sizeof(ENV_INSTANCE_DATA) * iCount);

	m_pContext->Unmap(m_pInstanceBuffer, 0);

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render()
{
	if (m_Submitted.empty())
		return S_OK;

	HRESULT hr = Should_Instance()
		? Render_Instanced()
		: Render_NotInstanced();

	Clear_Submissions();
	m_bRegisteredThisFrame = false;

	return hr;
}

HRESULT CEnv_InstanceBatch::Render_Shadow()
{
	if (m_Submitted.empty())
		return S_OK;

	HRESULT hr = Should_Instance()
		? Render_Shadow_Instanced()
		: Render_Shadow_NotInstanced();

	Clear_Submissions();
	m_bRegisteredThisFrame = false;

	return hr;
}

HRESULT CEnv_InstanceBatch::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	const ENV_INSTANCE_BATCH_DESC* pDesc = static_cast<ENV_INSTANCE_BATCH_DESC*>(pArg);
	m_tKey = pDesc->tKey;

	if (FAILED(Ready_Component()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Ready_Component()
{
	m_pShaderCom = Add_Component<CShader>(Shader_EnvInstance.iLevelID, Shader_EnvInstance.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_tKey.iModelProtoLevel, m_tKey.wstrModelProtoTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

void CEnv_InstanceBatch::BeginFrame_IfNeeded(_uint64 iCurrentFrame)
{
	if (m_iLastSubmitFrame == iCurrentFrame)
		return;

	Clear_Submissions();
	m_bRegisteredThisFrame = false;
	m_iLastSubmitFrame = iCurrentFrame;
}

_bool CEnv_InstanceBatch::Should_Instance() const
{
	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	if (iInstanceCount < INSTANCE_MIN_COUNT)
		return false;

	if (nullptr == m_pModelCom)
		return false;

	const _uint iMeshCount = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	const _uint iSaveDraws = iMeshCount * (iInstanceCount - 1);

	return iSaveDraws >= INSTANCE_MIN_SAVED_DRAWS;
}

HRESULT CEnv_InstanceBatch::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC))))
		return E_FAIL;

	return S_OK;
}

void CEnv_InstanceBatch::Clear_Submissions()
{
	for (auto& item : m_Submitted)
		Safe_Release(item.pObj);

	m_Submitted.clear();
}

HRESULT CEnv_InstanceBatch::Render_Instanced()
{
	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	if (0 == iInstanceCount)
		return S_OK;

	if (FAILED(Update_InstanceBuffer()))
		return E_FAIL;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX Layer = m_pModelCom->Get_MeshLayer(i);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)]);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, Layer.idx[ETOUI(MTEX_TYPE::NORMALS)]);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, Layer.idx[ETOUI(MTEX_TYPE::METALNESS)]);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_UnknownTexture", i, MTEX_TYPE::UNKNOWN, Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)]);

		const _uint iPass = (Layer.iPass >= 0)
			? static_cast<_uint>(Layer.iPass)
			: ShaderPass::NonAnimPBR::Diffuse;

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render_Instanced(i, m_pInstanceBuffer, sizeof(ENV_INSTANCE_DATA), iInstanceCount)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render_NotInstanced()
{
	for (auto& item : m_Submitted)
	{
		if (nullptr != item.pObj)
		{
			if (FAILED(item.pObj->Render()))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render_Shadow_Instanced()
{
	return Render_Shadow_NotInstanced();
}

HRESULT CEnv_InstanceBatch::Render_Shadow_NotInstanced()
{
	return S_OK;
}

CEnv_InstanceBatch* CEnv_InstanceBatch::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pDesc)
{
	CEnv_InstanceBatch* pInstance = new CEnv_InstanceBatch(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pDesc)))
	{
		MSG_BOX("Failed to Created : CEnv_InstanceBatch");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnv_InstanceBatch::Clone(void* pArg)
{
	CEnv_InstanceBatch* pInstance = new CEnv_InstanceBatch(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnv_InstanceBatch");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnv_InstanceBatch::Free()
{
	Clear_Submissions();

	Safe_Release(m_pInstanceBuffer);

	__super::Free();
}