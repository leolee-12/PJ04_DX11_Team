#include "Env_InstanceController.h"
#include "EnvObject_Static.h"
#include "Env_InstanceBatch.h"

#include "GameInstance_Proxy.h"

CEnv_InstanceController::CEnv_InstanceController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CEnv_InstanceController::CEnv_InstanceController(const CEnv_InstanceController& Prototype)
	: CGameObject(Prototype)
{
}

_bool CEnv_InstanceController::Submit_Main(CEnvObject_Static* pObj)
{
	if (nullptr == pObj)
		return false;

	const ENV_OBJECT_DESC& tDesc = pObj->Get_Desc();
	if (tDesc.strModelProtoTag.empty())
		return false;

	ENV_INSTANCE_KEY tKey{};
	tKey.iModelProtoLevel = tDesc.iModelProtoLevel;
	tKey.wstrModelProtoTag = tDesc.strModelProtoTag;
	tKey.eRenderID = RENDERID::NONBLEND;

	CEnv_InstanceBatch* pBatch = FindOrCreate_Batch(tKey);
	if (nullptr == pBatch)
	{
		return false;
	}

	const _uint64 iCurrentFrame = m_pGameInstance_Proxy->Get_FrameIndex();
	pBatch->Submit(pObj, iCurrentFrame);

	if (!pBatch->Is_RegisteredThisFrame())
	{
		pBatch->Set_RegisteredThisFrame(true);
		m_pGameInstance_Proxy->Add_RenderGroup(tKey.eRenderID, pBatch);
	}

	return true;
}

_bool CEnv_InstanceController::Submit_Shadow(CEnvObject_Static* pObj)
{
	if (nullptr == pObj)
		return false;

	const ENV_OBJECT_DESC& tDesc = pObj->Get_Desc();
	if (tDesc.strModelProtoTag.empty())
		return false;

	ENV_INSTANCE_KEY tKey{};
	tKey.iModelProtoLevel = tDesc.iModelProtoLevel;
	tKey.wstrModelProtoTag = tDesc.strModelProtoTag;
	tKey.eRenderID = RENDERID::SHADOW;

	CEnv_InstanceBatch* pBatch = FindOrCreate_Batch(tKey);
	if (nullptr == pBatch)
		return false;

	const _uint64 iCurrentFrame = m_pGameInstance_Proxy->Get_FrameIndex();
	pBatch->Submit(pObj, iCurrentFrame);

	if (!pBatch->Is_RegisteredThisFrame())
	{
		pBatch->Set_RegisteredThisFrame(true);
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, pBatch);
	}
	
	return true;
}

CEnv_InstanceBatch* CEnv_InstanceController::FindOrCreate_Batch(const ENV_INSTANCE_KEY& tKey)
{
	auto iter = m_Batches.find(tKey);
	if (iter != m_Batches.end())
		return iter->second;

	CEnv_InstanceBatch::ENV_INSTANCE_BATCH_DESC tDesc{};
	tDesc.tKey = tKey;

	CEnv_InstanceBatch* pBatch = CEnv_InstanceBatch::Create(m_pDevice, m_pContext, &tDesc);
	if (nullptr == pBatch)
		return nullptr;

	m_Batches.emplace(tKey, pBatch);
	return pBatch;
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
	for (auto& pair : m_Batches)
		Safe_Release(pair.second);

	m_Batches.clear();

	__super::Free();
}
