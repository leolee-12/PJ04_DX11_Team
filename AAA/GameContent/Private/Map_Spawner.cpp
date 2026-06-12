#include "Map_Spawner.h"
#include "MapStage.h"
#include "EnvObject_Static.h"
#include "EnvObject_Interact.h"
#include "EnvObject_Trigger.h"

#include "GameInstance.h"

namespace
{
	struct PENDING_CREATED_CALLBACK_INFO
	{
		CGameObject* pObject = nullptr;
		_wstring        strPrototypeTag;
		_wstring        strLayerTag;
		_wstring        strObjectTag;
	};
}

CMap_Spawner::CMap_Spawner()
	: m_pProxy{ CGameInstance::GetProxy() }
{
}

HRESULT CMap_Spawner::Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_REPORT* pOutReport)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	const MAP_RUNTIME_LEVELS& Levels = Request.Levels;
	const MAP_SPAWN_TARGETS& Targets = Request.Targets;
	const MAP_SPAWN_OPTIONS& Options = Request.Options;
	MAP_OBJECT_CREATED_CALLBACK pCreatedCallback = Request.pCreatedCallback;
	void* pCallbackContext = Request.pCallbackContext;
	CMapStage** ppOutStage = Request.ppOutStage;

	if (!Options.bSpawnStage && !Options.bSpawnEnv)
		return E_FAIL;

	if (nullptr != pOutReport)
		*pOutReport = {};

	if (nullptr != ppOutStage)
		*ppOutStage = nullptr;

	MAP_STAGE_DESC StageDesc = Package.StageDesc;
	StageDesc.iSectionProtoLevel = Levels.iObjectLevel;

	for (MAP_SECTION_DESC& SectionDesc : StageDesc.SectionDescs)
		SectionDesc.iModelProtoLevel = Levels.iStageModelLevel;

	vector<CGameObject*> CreatedObjects;
	vector<PENDING_CREATED_CALLBACK_INFO> PendingCallbacks;
	CMapStage* pStage = nullptr;

	if (Options.bSpawnStage)
	{
		if (nullptr == Targets.Stage.pLayerTag || nullptr == Targets.pStageObjectTag)
			return E_FAIL;

		CGameObject* pStageObject = nullptr;
		if (FAILED(m_pProxy->Add_GameObject_Return(
			&pStageObject,
			Levels.iObjectLevel,
			CMapStage::PROTOTYPE_TAG,
			Targets.Stage.iPlaceLevel,
			Targets.Stage.pLayerTag,
			Targets.pStageObjectTag,
			&StageDesc)))
		{
			return E_FAIL;
		}

		pStage = dynamic_cast<CMapStage*>(pStageObject);
		if (nullptr == pStage)
		{
			if (nullptr != pStageObject)
				m_pProxy->Destroy_GameObject(pStageObject);
			return E_FAIL;
		}

		CreatedObjects.push_back(pStageObject);
		PendingCallbacks.push_back({
			pStageObject,
			CMapStage::PROTOTYPE_TAG,
			Targets.Stage.pLayerTag,
			Targets.pStageObjectTag
			});
	}

	if (Options.bSpawnEnv)
	{
		for (const ENV_OBJECT_DESC& SrcDesc : Package.EnvObjectDescs)
		{
			const MAP_SPAWN_ROUTE* pRoute = Resolve_EnvRoute(Targets, SrcDesc.eKind);
			if (nullptr == pRoute || nullptr == pRoute->pLayerTag)
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			const _tchar* pProtoTag = Get_EnvObjectProtoTag(SrcDesc.eKind);
			if (nullptr == pProtoTag)
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			ENV_OBJECT_DESC Desc = SrcDesc;
			Desc.iModelProtoLevel = Levels.iEnvModelLevel;

			CGameObject* pCreatedObject = nullptr;
			const wstring strObjectName = Make_EnvObjectName(Desc);

			if (FAILED(m_pProxy->Add_GameObject_Return(
				&pCreatedObject,
				Levels.iObjectLevel,
				pProtoTag,
				pRoute->iPlaceLevel,
				pRoute->pLayerTag,
				strObjectName,
				&Desc)))
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			CreatedObjects.push_back(pCreatedObject);
			PendingCallbacks.push_back({
				pCreatedObject,
				pProtoTag,
				pRoute->pLayerTag,
				strObjectName
				});
		}
	
		for (const MAP_ADDED_OBJECT_DESC& Added : Package.AddedObjectDescs)
		{
			CGameObject* pCreatedObject = nullptr;

			if (FAILED(m_pProxy->Add_GameObject_Return(
				&pCreatedObject,
				Levels.iObjectLevel,
				Added.strPrototypeTag.c_str(),
				Levels.iObjectLevel,
				Added.strLayerTag.c_str(),
				Added.strObjectTag,
				nullptr)))
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			if (nullptr != pCreatedObject)
				pCreatedObject->Deserialize(Added.jObject);

			CreatedObjects.push_back(pCreatedObject);
			PendingCallbacks.push_back({
				pCreatedObject,
				Added.strPrototypeTag,
				Added.strLayerTag,
				Added.strObjectTag
				});
		}
	}

	if (nullptr != pOutReport)
	{
		pOutReport->iSectionCount = static_cast<_uint>(Package.StageDesc.SectionDescs.size());

		if (Options.bSpawnStage)
		{
			pOutReport->bStageLoaded = true;
			pOutReport->strStageName = StageDesc.strStageName;
		}

		if (Options.bSpawnEnv)
		{
			pOutReport->iEnvDescriptorCount = static_cast<_uint>(Package.EnvObjectDescs.size());
			pOutReport->iEnvCreatedCount = static_cast<_uint>(Package.EnvObjectDescs.size());
			pOutReport->iEnvJsonLoadedCount = static_cast<_uint>(Package.EnvJsonPaths.size());
		}
	}

	if (nullptr != ppOutStage && Options.bSpawnStage)
		*ppOutStage = pStage;

	if (nullptr != pCreatedCallback)
	{
		for (const auto& Info : PendingCallbacks)
		{
			pCreatedCallback(
				pCallbackContext,
				Info.pObject,
				Info.strPrototypeTag,
				Info.strLayerTag,
				Info.strObjectTag);
		}
	}

	return S_OK;
}

const MAP_SPAWN_ROUTE* CMap_Spawner::Resolve_EnvRoute(const MAP_SPAWN_TARGETS& Targets,
	ENV_OBJECT_KIND eKind) const
{
	switch (eKind)
	{
	case ENV_OBJECT_KIND::STATIC: return &Targets.EnvStatic;
	case ENV_OBJECT_KIND::INTERACT: return &Targets.EnvInteract;
	case ENV_OBJECT_KIND::EFFECT: return &Targets.EnvEffect;
	default: return nullptr;
	}
}

const _tchar* CMap_Spawner::Get_EnvObjectProtoTag(ENV_OBJECT_KIND eKind) const
{
	switch (eKind)
	{
	case ENV_OBJECT_KIND::STATIC: return CEnvObject_Static::PROTOTYPE_TAG;
	case ENV_OBJECT_KIND::INTERACT: return CEnvObject_Interact::PROTOTYPE_TAG;
	case ENV_OBJECT_KIND::EFFECT: return CEnvObject_Trigger::PROTOTYPE_TAG;
	default: return nullptr;
	}
}

_wstring CMap_Spawner::Make_EnvObjectName(const ENV_OBJECT_DESC& Desc) const
{
	wstring strName = L"Env_" + Desc.strObjectName;
	if (0 != Desc.iUid)
		strName += L"_" + to_wstring(Desc.iUid);
	else if (!Desc.strEntryKey.empty())
		strName += L"_" + Desc.strEntryKey;
	return strName;
}

void CMap_Spawner::Rollback(const vector<CGameObject*>& CreatedObjects)
{
	if (nullptr == m_pProxy)
		return;

	for (auto iter = CreatedObjects.rbegin(); iter != CreatedObjects.rend(); ++iter)
	{
		if (nullptr != *iter)
			m_pProxy->Destroy_GameObject(*iter);
	}
}

CMap_Spawner* CMap_Spawner::Create()
{
	return new CMap_Spawner();
}

void CMap_Spawner::Free()
{
	Safe_Release(m_pProxy);

	__super::Free();
}