#include "Env_InstanceBatch.h"
#include "MeshLayer_Binder.h"
#include "EnvObject_Static.h"

#include "GameInstance.h"
#include "Profiler_Manager.h"

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

	tData.InstanceData.matWorld = *pObj->Get_Transform()->Get_WorldMatrixPtr();
	tData.InstanceData.vDissolveParams =
	{
		pObj->Get_FinalMainDissolve(),
		pObj->Get_FinalShadowDissolve(),
		pObj->Get_NearDitherLength(),
		pObj->Get_DecalAlpha()
	};

	m_Submitted.push_back(tData);
}

HRESULT CEnv_InstanceBatch::Reserve_InstanceBuffer(_uint iRequiredCount)
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

	if (FAILED(Reserve_InstanceBuffer(iCount)))
		return E_FAIL;

	D3D11_MAPPED_SUBRESOURCE tMapped{};
	if(FAILED(m_pContext->Map(m_pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &tMapped)))
		return E_FAIL;

	ENV_INSTANCE_DATA* pDst = static_cast<ENV_INSTANCE_DATA*>(tMapped.pData);

	for (_uint i = 0; i < iCount; ++i)
	{
		pDst[i] = m_Submitted[i].InstanceData;
	}

	m_pContext->Unmap(m_pInstanceBuffer, 0);

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render()
{
	if (m_Submitted.empty())
		return S_OK;

	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	const _bool bUseInstancing = Should_Instance();

	HRESULT hr = bUseInstancing
		? Render_Instanced()
		: Render_NotInstanced();

	if (SUCCEEDED(hr))
	{
		PROFILE_COUNTER_ADD(
			bUseInstancing
			? Engine::EPROFILE_COUNTER::ENV_INSTANCE_RENDERED
			: Engine::EPROFILE_COUNTER::ENV_INSTANCE_FALLBACK,
			iInstanceCount);
	}

	Clear_Submissions();
	m_bRegisteredThisFrame = false;

	return hr;
}

HRESULT CEnv_InstanceBatch::Render_Shadow()
{
	if (m_Submitted.empty())
		return S_OK;

	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	const _bool bUseInstancing = Should_Instance();

	HRESULT hr = bUseInstancing
		? Render_Shadow_Instanced()
		: Render_Shadow_NotInstanced();

	if (SUCCEEDED(hr))
	{
		PROFILE_COUNTER_ADD(
			bUseInstancing
			? Engine::EPROFILE_COUNTER::ENV_INSTANCE_RENDERED
			: Engine::EPROFILE_COUNTER::ENV_INSTANCE_FALLBACK,
			iInstanceCount);
	}

	Clear_Submissions();
	m_bRegisteredThisFrame = false;

	return hr;
}

HRESULT CEnv_InstanceBatch::Render_Decal()
{
	if (m_Submitted.empty())
		return S_OK;

	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	const _bool bUseInstancing = Should_Instance();

	const HRESULT hr = bUseInstancing
		? Render_Decal_Instanced()
		: Render_Decal_NotInstanced();

	if (SUCCEEDED(hr))
	{
		PROFILE_COUNTER_ADD(
			bUseInstancing
			? Engine::EPROFILE_COUNTER::ENV_INSTANCE_RENDERED
			: Engine::EPROFILE_COUNTER::ENV_INSTANCE_FALLBACK,
			iInstanceCount);
	}

	Clear_Submissions();
	m_bRegisteredThisFrame = false;

	return hr;
}

HRESULT CEnv_InstanceBatch::Apply_MeshLayer(_uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (nullptr == m_pModelCom || iMesh >= m_pModelCom->Get_NumMeshes())
		return E_FAIL;

	m_pModelCom->Set_MeshLayer(iMesh, Layer);
	return S_OK;
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
	m_pShaderCom = Add_Component<CShader>(Shader_World_Instance.iLevelID, Shader_World_Instance.szProtoTag, TEXT("Com_Shader"));
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

	const _uint iMaterialID = WORLD_STATIC_ID;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Bind_ShadowShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
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
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_INSTANCE;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::DMN);

		_uint iPass = 0u;
		const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
		if (FAILED(hrBind))     return E_FAIL;
		if (S_FALSE == hrBind)  continue;

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

HRESULT CEnv_InstanceBatch::Render_Decal_Instanced()
{
	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	if (0 == iInstanceCount)
		return S_OK;

	CEnvObject_Static* pReferenceObj = m_Submitted.front().pObj;
	if (nullptr == pReferenceObj)
		return Render_Decal_NotInstanced();

	if (FAILED(Update_InstanceBuffer()))
		return E_FAIL;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
		return E_FAIL;

	const BoundingBox& LocalBounds = pReferenceObj->Get_LocalBounds();
	const _float3 vDecalBoundsCenter = LocalBounds.Center;
	const _float3 vDecalBoundsExtents = LocalBounds.Extents;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vDecalBoundsCenter", &vDecalBoundsCenter, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vDecalBoundsExtents", &vDecalBoundsExtents, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_MaterialID"), m_pShaderCom, "g_MaterialIDTexture")))
		return E_FAIL;

	const _int iDecalMaskMode = 1;
	const _int iDecalMaskID = WORLD_STATIC_ID;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iDecalMaskMode", &iDecalMaskMode, sizeof(_int)))) return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iDecalMaskID", &iDecalMaskID, sizeof(_int)))) return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_INSTANCE;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::DECAL;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::DECAL);

		_uint iPass = 0u;
		const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
		if (FAILED(hrBind))     return E_FAIL;
		if (S_FALSE == hrBind)  continue;

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render_Instanced(i, m_pInstanceBuffer, sizeof(ENV_INSTANCE_DATA), iInstanceCount)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render_Decal_NotInstanced()
{
	for (auto& item : m_Submitted)
	{
		if (nullptr != item.pObj)
		{
			if (FAILED(item.pObj->Render_Decal()))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render_Shadow_Instanced()
{
	const _uint iInstanceCount = static_cast<_uint>(m_Submitted.size());
	if (0 == iInstanceCount)
		return S_OK;

	if (FAILED(Update_InstanceBuffer()))
		return E_FAIL;

	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_INSTANCE;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::SHADOW;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::SHADOW);

		_uint iPass = 0u;
		const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
		if (FAILED(hrBind))     return E_FAIL;
		if (S_FALSE == hrBind)  continue;

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render_Instanced(i, m_pInstanceBuffer, sizeof(ENV_INSTANCE_DATA), iInstanceCount)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEnv_InstanceBatch::Render_Shadow_NotInstanced()
{
	for (auto& item : m_Submitted)
	{
		if (nullptr != item.pObj)
		{
			if (FAILED(item.pObj->Render_Shadow()))
				return E_FAIL;
		}
	}

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