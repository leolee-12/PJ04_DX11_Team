#include "Env_InstanceController.h"
#include "EnvObject_Static.h"
#include "Env_InstanceBatch.h"

#include "GameInstance_Proxy.h"
#include "Profiler_Manager.h"

CEnv_InstanceController::CEnv_InstanceController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CEnv_InstanceController::CEnv_InstanceController(const CEnv_InstanceController& Prototype)
	: CGameObject(Prototype)
{
}

ENV_INSTANCE_BATCH_HANDLE CEnv_InstanceController::Register_BatchesForDesc(const ENV_OBJECT_DESC& tDesc)
{
	ENV_INSTANCE_BATCH_HANDLE tHandle{};

	if (tDesc.wstrModelProtoTag.empty())
		return tHandle;

	if (tDesc.tRender.bIsDecal)
	{
		ENV_INSTANCE_KEY tDecalKey{};
		tDecalKey.iModelProtoLevel = tDesc.iModelProtoLevel;
		tDecalKey.wstrModelProtoTag = tDesc.wstrModelProtoTag;
		tDecalKey.eRenderID = RENDERID::DECAL;
		tHandle.iDecalBatchIndex = FindOrCreate_BatchIndex(tDecalKey);

		return tHandle;
	}

	ENV_INSTANCE_KEY tMainKey{};
	tMainKey.iModelProtoLevel = tDesc.iModelProtoLevel;
	tMainKey.wstrModelProtoTag = tDesc.wstrModelProtoTag;
	tMainKey.eRenderID = RENDERID::NONBLEND;
	tHandle.iMainBatchIndex = FindOrCreate_BatchIndex(tMainKey);

	return tHandle;
}

_uint CEnv_InstanceController::Register_ShadowBatch(const ENV_OBJECT_DESC& tDesc)
{
	if (tDesc.wstrModelProtoTag.empty() || tDesc.tRender.bIsDecal)
		return INVALID_INDEX;

	ENV_INSTANCE_KEY tShadowKey{};
	tShadowKey.iModelProtoLevel = tDesc.iModelProtoLevel;
	tShadowKey.wstrModelProtoTag = tDesc.wstrModelProtoTag;
	tShadowKey.eRenderID = RENDERID::SHADOW;

	return FindOrCreate_BatchIndex(tShadowKey);
}

_bool CEnv_InstanceController::Submit_Main(_uint iBatchIndex, CEnvObject_Static* pObj)
{
	if (nullptr == pObj)
		return false;

	if (INVALID_INDEX == iBatchIndex || iBatchIndex >= m_Batches.size())
		return false;

	CEnv_InstanceBatch* pBatch = m_Batches[iBatchIndex];
	if (nullptr == pBatch)
		return false;

	const _uint64 iCurrentFrame = m_pGameInstance_Proxy->Get_FrameIndex();
	pBatch->Submit(pObj, iCurrentFrame);
	PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_INSTANCE_SUBMITTED, 1);

	if (!pBatch->Is_RegisteredThisFrame())
	{
		pBatch->Set_RegisteredThisFrame(true);
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, pBatch);
		PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_INSTANCE_BATCH_SUBMITTED, 1);
	}

	return true;
}

_bool CEnv_InstanceController::Submit_Shadow(_uint iBatchIndex, CEnvObject_Static* pObj)
{
	if (nullptr == pObj)
		return false;

	if (INVALID_INDEX == iBatchIndex || iBatchIndex >= m_Batches.size())
		return false;

	CEnv_InstanceBatch* pBatch = m_Batches[iBatchIndex];
	if (nullptr == pBatch)
		return false;

	const _uint64 iCurrentFrame = m_pGameInstance_Proxy->Get_FrameIndex();
	pBatch->Submit(pObj, iCurrentFrame);
	PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_INSTANCE_SUBMITTED, 1);

	if (!pBatch->Is_RegisteredThisFrame())
	{
		pBatch->Set_RegisteredThisFrame(true);
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pBatch);
		PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_INSTANCE_BATCH_SUBMITTED, 1);
	}

	return true;
}

_bool CEnv_InstanceController::Submit_Decal(_uint iBatchIndex, CEnvObject_Static* pObj)
{
	if (nullptr == pObj)
		return false;

	if (INVALID_INDEX == iBatchIndex || iBatchIndex >= m_Batches.size())
		return false;

	CEnv_InstanceBatch* pBatch = m_Batches[iBatchIndex];
	if (nullptr == pBatch)
		return false;

	const _uint64 iCurrentFrame = m_pGameInstance_Proxy->Get_FrameIndex();
	pBatch->Submit(pObj, iCurrentFrame);
	PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_INSTANCE_SUBMITTED, 1);

	if (!pBatch->Is_RegisteredThisFrame())
	{
		pBatch->Set_RegisteredThisFrame(true);
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::DECAL, pBatch);
		PROFILE_COUNTER_ADD(Engine::EPROFILE_COUNTER::ENV_INSTANCE_BATCH_SUBMITTED, 1);
	}

	return true;
}

HRESULT CEnv_InstanceController::Apply_ModelMeshLayer(_uint iModelProtoLevel, const _wstring& wstrModelProtoTag, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (wstrModelProtoTag.empty())
		return S_OK;

	const RENDERID RenderIDs[] =
	{
			RENDERID::NONBLEND,
			RENDERID::SHADOW,
			RENDERID::DECAL
	};

	for (const RENDERID eRenderID : RenderIDs)
	{
		ENV_INSTANCE_KEY tKey{};
		tKey.iModelProtoLevel = iModelProtoLevel;
		tKey.wstrModelProtoTag = wstrModelProtoTag;
		tKey.eRenderID = eRenderID;

		CEnv_InstanceBatch* pBatch = Find_Batch(tKey);
		if (nullptr == pBatch)
			continue;

		if (FAILED(pBatch->Apply_MeshLayer(iMesh, Layer)))
			return E_FAIL;
	}

	m_MeshLayerOverridesByModel[Make_ModelMeshLayerKey(iModelProtoLevel, wstrModelProtoTag)][iMesh] = Layer;

		return S_OK;
}

CEnv_InstanceBatch* CEnv_InstanceController::Find_Batch(const ENV_INSTANCE_KEY& tKey)
{
	const auto Iter = m_BatchIndexByKey.find(tKey);
	if (Iter == m_BatchIndexByKey.end())
		return nullptr;

	const _uint iBatchIndex = Iter->second;
	if (iBatchIndex >= m_Batches.size())
		return nullptr;

	return m_Batches[iBatchIndex];
}

_uint CEnv_InstanceController::FindOrCreate_BatchIndex(const ENV_INSTANCE_KEY& tKey)
{
	auto iter = m_BatchIndexByKey.find(tKey);
	if (iter != m_BatchIndexByKey.end())
		return iter->second;

	CEnv_InstanceBatch::ENV_INSTANCE_BATCH_DESC tDesc{};
	tDesc.tKey = tKey;

	CEnv_InstanceBatch* pBatch = CEnv_InstanceBatch::Create(m_pDevice, m_pContext, &tDesc);
	if (nullptr == pBatch)
		return INVALID_INDEX;

	if (FAILED(Apply_CachedMeshLayers(tKey, pBatch)))
	{
		Safe_Release(pBatch);
		return INVALID_INDEX;
	}

	const _uint iBatchIndex = static_cast<_uint>(m_Batches.size());
	m_Batches.push_back(pBatch);
	m_BatchIndexByKey.emplace(tKey, iBatchIndex);
	return iBatchIndex;
}


HRESULT CEnv_InstanceController::Apply_CachedMeshLayers(const ENV_INSTANCE_KEY& tBatchKey, CEnv_InstanceBatch* pBatch)
{
	if (nullptr == pBatch)
		return E_FAIL;

	const auto Iter = m_MeshLayerOverridesByModel.find(
		Make_ModelMeshLayerKey(tBatchKey.iModelProtoLevel, tBatchKey.wstrModelProtoTag));

	if (Iter == m_MeshLayerOverridesByModel.end())
		return S_OK;

	const unordered_map<_uint, MESH_LAYER_IDX>& Layers = Iter->second;
	for (const auto& [iMesh, Layer] : Layers)
	{
		if (FAILED(pBatch->Apply_MeshLayer(iMesh, Layer)))
			return E_FAIL;
	}

	return S_OK;
}

ENV_INSTANCE_KEY CEnv_InstanceController::Make_ModelMeshLayerKey(_uint iModelProtoLevel, const _wstring& wstrModelProtoTag)
{
	ENV_INSTANCE_KEY tKey{};
	tKey.iModelProtoLevel = iModelProtoLevel;
	tKey.wstrModelProtoTag = wstrModelProtoTag;
	tKey.eRenderID = RENDERID::NONBLEND;
	return tKey;
}

CEnv_InstanceController* CEnv_InstanceController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnv_InstanceController* pInstance = new CEnv_InstanceController(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnv_InstanceController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnv_InstanceController::Clone(void* pArg)
{
	CEnv_InstanceController* pInstance = new CEnv_InstanceController(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnv_InstanceController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnv_InstanceController::Free()
{
	for (CEnv_InstanceBatch* pBatch : m_Batches)
		Safe_Release(pBatch);

	m_Batches.clear();
	m_BatchIndexByKey.clear();

	__super::Free();
}
