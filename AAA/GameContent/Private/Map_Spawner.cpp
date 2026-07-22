#include "Map_Spawner.h"
#include "MapStage.h"
#include "MapGimmickSection.h"
#include "EnvObject_Static.h"
#include "EnvObject_Interact.h"
#include "EnvInteract_KickProp.h"
#include "EnvTrigger_Generic.h"
#include "EnvTrigger_RenderGlobals.h"
#include "EnvTrigger_EventPublisher.h"
#include "EnvVolume_Effect.h"
#include "EnvVolume_Culling.h"
#include "EnvVolume_Light.h"
#include "Env_SpotLight.h"
#include "Env_InstanceController.h"
#include "World_BlendCollector.h"
#include "LevelDesign_Registry.h"
#include "GameContent_Log.h"
#include "GameObject_Factory.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"

namespace
{
	static constexpr const _tchar* ENV_INSTANCE_LAYER_TAG = L"Layer_EnvInstance";
	static constexpr const _tchar* ENV_INSTANCE_OBJECT_TAG = L"Env_InstanceController";

	struct PENDING_CREATED_CALLBACK_INFO
	{
		CGameObject*	pObject = nullptr;
		_wstring		strPrototypeTag;
		_wstring		strLayerTag;
		_wstring		strObjectTag;
	};

	_bool Is_AddedEnvRuntimePrototype(const _wstring& strPrototypeTag)
	{
		return strPrototypeTag == CEnvTrigger_Generic::PROTOTYPE_TAG
			|| strPrototypeTag == CEnvTrigger_Generic::LEGACY_PROTOTYPE_TAG
			|| strPrototypeTag == CEnvTrigger_RenderGlobals::PROTOTYPE_TAG
			|| strPrototypeTag == CEnvTrigger_EventPublisher::PROTOTYPE_TAG
			|| strPrototypeTag == CEnvVolume_Effect::PROTOTYPE_TAG
			|| strPrototypeTag == CEnvVolume_Culling::PROTOTYPE_TAG
			|| strPrototypeTag == CEnvVolume_Light::PROTOTYPE_TAG
			|| strPrototypeTag == CEnv_SpotLight::PROTOTYPE_TAG;
	}

	enum class ADDED_MAP_OBJECT_FAMILY
	{
		UNKNOWN,
		ENV,
		LEVELDESIGN
	};

	ADDED_MAP_OBJECT_FAMILY Resolve_AddedMapObjectFamily(const _wstring& strPrototypeTag)
	{
		if (Is_AddedEnvRuntimePrototype(strPrototypeTag))
			return ADDED_MAP_OBJECT_FAMILY::ENV;

		const auto* pRegistration = CGameObject_Factory::GetInstance()->Get_Registration(strPrototypeTag);
		if (nullptr == pRegistration)
			return ADDED_MAP_OBJECT_FAMILY::UNKNOWN;

		if (pRegistration->strCategory == L"ENV_TRIGGER")
			return ADDED_MAP_OBJECT_FAMILY::ENV;

		if (pRegistration->strCategory == L"LEVELDESIGN_OBJECT"
			|| pRegistration->strCategory == L"DEFORM_OBJECT")
		{
			return ADDED_MAP_OBJECT_FAMILY::LEVELDESIGN;
		}

		return ADDED_MAP_OBJECT_FAMILY::UNKNOWN;
	}

	ENV_OBJECT_DESC Make_AddedEnvRuntimeDesc(const MAP_ADD_OBJECT& Added)
	{
		ENV_OBJECT_DESC Desc{};
		Desc.eKind = ENV_OBJECT_KIND::EFFECT;
		Desc.wstrObjectName = Added.strObjectTag;
		Desc.wstrComponentName = Added.strObjectTag;
		Desc.vScale = { 1.f, 1.f, 1.f };
		Desc.vRotation = { 0.f, 0.f, 0.f, 1.f };
		Desc.tCollision.bInvisibleCollision = true;
		Desc.tRender.bUseCullDistance = false;
		Desc.tRender.bUseCullFrustum = false;
		Desc.tRender.bHasShadow = false;

		if (Added.strPrototypeTag == CEnvTrigger_RenderGlobals::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::TONE_MAPPING_AREA;
		else if (Added.strPrototypeTag == CEnvVolume_Effect::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::FIELD_EFFECT;
		else if (Added.strPrototypeTag == CEnvVolume_Culling::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::DECOR_PARTS_CULLING_AREA;
		else if (Added.strPrototypeTag == CEnvVolume_Light::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::LOCAL_AREA_LIGHT;
		else if (Added.strPrototypeTag == CEnv_SpotLight::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::SPOT_LIGHT;

		JsonUtils::Try_ReadFloat3Array(Added.jObject, "Area Center", &Desc.tEffect.vAreaCenter);
		if (false == JsonUtils::Try_ReadFloat3Array(Added.jObject, "Area Size", &Desc.tEffect.vAreaSize))
			Desc.tEffect.vAreaSize = { 1.f, 1.f, 1.f };

		JsonUtils::Try_ReadFloat4Array(Added.jObject, "Area Rotation", &Desc.tEffect.vAreaRot);

		return Desc;
	}
}

CMap_Spawner::CMap_Spawner()
	: m_pProxy{ CGameInstance::GetProxy() }
{
}

HRESULT CMap_Spawner::Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_RESULT* pOutReport)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	const MAP_RUNTIME_LEVELS& Levels = Request.Levels;
	const MAP_SPAWN_TARGETS& Targets = Request.Targets;
	const MAP_SPAWN_OPTIONS& Options = Request.Options;
	const _bool bNeedWorldBlendCollector = Options.bSpawnEnv || Options.bSpawnAddedLevelDesign;
	MAP_OBJECT_CREATED_CALLBACK pCreatedCallback = Request.pCreatedCallback;
	void* pCallbackContext = Request.pCallbackContext;
	CMapStage** ppOutStage = Request.ppOutStage;
	CEnv_InstanceController** ppOutEnvInstanceController = Request.ppOutEnvInstanceController;

	if (!Options.bSpawnStage && !Options.bSpawnEnv
		&& !Options.bSpawnAddedEnv && !Options.bSpawnAddedLevelDesign)
	{
		return E_FAIL;
	}

	if (nullptr != pOutReport)
		*pOutReport = {};

	if (nullptr != ppOutStage)
		*ppOutStage = nullptr;

	if (nullptr != ppOutEnvInstanceController)
		*ppOutEnvInstanceController = nullptr;

	vector<CGameObject*> CreatedObjects;
	vector<PENDING_CREATED_CALLBACK_INFO> PendingCallbacks;

	size_t iGimmickObjectCount = 0;

	if (Options.bSpawnStage)
	{
		For_Each_MapGimmickEntry(Package.StageDesc.strStageName,
			[&](const MAP_GIMMICK_SECTION_ENTRY&)
			{
				++iGimmickObjectCount;
			});
	}

	const size_t iExpectedObjectCount =
		(Options.bSpawnStage ? 1u + iGimmickObjectCount : 0u)
		+ (Options.bSpawnEnv ? Package.EnvObjectDescs.size() + 1u : 0u)
		+ ((Options.bSpawnAddedEnv || Options.bSpawnAddedLevelDesign)
			? Package.AddedObjectDescs.size()
			: 0u)
		+ (bNeedWorldBlendCollector ? 1u : 0u);

	CreatedObjects.reserve(iExpectedObjectCount);
	PendingCallbacks.reserve(iExpectedObjectCount);

	CMapStage* pStage = nullptr;
	CEnv_InstanceController* pEnvInstanceController = nullptr;

	if (bNeedWorldBlendCollector && nullptr == CWorld_BlendCollector::Find(m_pProxy))
	{
		CGameObject* pCollectorObject = nullptr;

		if (FAILED(m_pProxy->Add_GameObject_Return(
			&pCollectorObject,
			ETOUI(LEVEL::STATIC),
			CWorld_BlendCollector::PROTOTYPE_TAG,
			ETOUI(LEVEL::STATIC),
			CWorld_BlendCollector::LAYER_TAG,
			CWorld_BlendCollector::OBJECT_TAG,
			nullptr)))
		{
			Rollback(CreatedObjects);
			return E_FAIL;
		}

		if (nullptr == dynamic_cast<CWorld_BlendCollector*>(pCollectorObject))
		{
			if (nullptr != pCollectorObject)
				m_pProxy->Destroy_GameObject(pCollectorObject);

			Rollback(CreatedObjects);
			return E_FAIL;
		}

		CreatedObjects.push_back(pCollectorObject);
	}

	if (Options.bSpawnStage)
	{
		if (nullptr == Targets.Stage.pLayerTag || nullptr == Targets.pStageObjectTag)
			return E_FAIL;

		MAP_STAGE_DESC StageDesc = Package.StageDesc;
		StageDesc.iSectionProtoLevel = Levels.iObjectLevel;

		for (MAP_SECTION_DESC& SectionDesc : StageDesc.SectionDescs)
			SectionDesc.iModelProtoLevel = Levels.iStageModelLevel;

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

		for (const MAP_GIMMICK_SECTION_ENTRY& Entry : g_MapGimmickSections)
		{
			if (StageDesc.strStageName != Entry.pStageName)
				continue;

			CMapGimmickSection::MAP_GIMMICK_SECTION_DESC Desc{};
			Desc.pEntry = &Entry;
			Desc.iModelProtoLevel = Levels.iStageModelLevel;
			Desc.bRenderable = false;

			CGameObject* pGimmickSection = nullptr;
			if (FAILED(m_pProxy->Add_GameObject_Return(
				&pGimmickSection,
				Levels.iObjectLevel,
				CMapGimmickSection::PROTOTYPE_TAG,
				Targets.Stage.iPlaceLevel,
				CMapGimmickSection::LAYER_TAG,
				Entry.pObjectTag,
				&Desc)))
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			CreatedObjects.push_back(pGimmickSection);
			PendingCallbacks.push_back({
					pGimmickSection,
					CMapGimmickSection::PROTOTYPE_TAG,
					CMapGimmickSection::LAYER_TAG,
					Entry.pObjectTag
				});
		}
	}

	_uint iEnvCreatedCount = 0;
	_uint iEnvSkippedCreateFailed = 0;

	if (Options.bSpawnEnv)
	{
		const auto iterStaticEnv = find_if(
			Package.EnvObjectDescs.begin(),
			Package.EnvObjectDescs.end(),
			[](const ENV_OBJECT_DESC& Desc) -> _bool
			{
				return Desc.eKind == ENV_OBJECT_KIND::STATIC;
			});

		if (iterStaticEnv != Package.EnvObjectDescs.end())
		{
			CGameObject* pControllerObject = nullptr;

			if (FAILED(m_pProxy->Add_GameObject_Return(
				&pControllerObject,
				ETOUI(LEVEL::STATIC),
				CEnv_InstanceController::PROTOTYPE_TAG,
				Targets.EnvStatic.iPlaceLevel,
				ENV_INSTANCE_LAYER_TAG,
				ENV_INSTANCE_OBJECT_TAG,
				nullptr)))
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			pEnvInstanceController = dynamic_cast<CEnv_InstanceController*>(pControllerObject);
			if (nullptr == pEnvInstanceController)
			{
				if (nullptr != pControllerObject)
					m_pProxy->Destroy_GameObject(pControllerObject);

				Rollback(CreatedObjects);
				return E_FAIL;
			}

			CreatedObjects.push_back(pControllerObject);
		}

		for (const ENV_OBJECT_DESC& SrcDesc : Package.EnvObjectDescs)
		{
			const MAP_SPAWN_ROUTE* pRoute = Resolve_EnvRoute(Targets, SrcDesc.eKind);
			if (nullptr == pRoute || nullptr == pRoute->pLayerTag)
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			const _tchar* pProtoTag = Get_EnvObjectProtoTag(SrcDesc);
			if (nullptr == pProtoTag)
			{
				Rollback(CreatedObjects);
				return E_FAIL;
			}

			ENV_OBJECT_DESC Desc = SrcDesc;
			Desc.iModelProtoLevel = Levels.iEnvModelLevel;

			if (EnvObject_NeedsModel(SrcDesc) && (Desc.wstrModelProtoTag.empty()
					|| !m_pProxy->Has_Prototype(Levels.iEnvModelLevel, Desc.wstrModelProtoTag)))
			{
				++iEnvSkippedCreateFailed;
				continue;
			}

			CGameObject* pCreatedObject = nullptr;
			const wstring wstrObjectName = Make_EnvObjectName(Desc);

			if (FAILED(m_pProxy->Add_GameObject_Return(
				&pCreatedObject,
				Levels.iObjectLevel,
				pProtoTag,
				pRoute->iPlaceLevel,
				pRoute->pLayerTag,
				wstrObjectName,
				&Desc)))
			{
				if (nullptr != pCreatedObject)
					m_pProxy->Destroy_GameObject(pCreatedObject);

				++iEnvSkippedCreateFailed;

				Log_GameContentWarning(
					"EnvObject spawn skipped: object="
					+ WstrToStr(Desc.wstrObjectName)
					+ " model=" + WstrToStr(Desc.wstrModelPath));

				continue;
			}

			++iEnvCreatedCount;
			CreatedObjects.push_back(pCreatedObject);
			PendingCallbacks.push_back({ pCreatedObject, pProtoTag, pRoute->pLayerTag, wstrObjectName });


			if (SrcDesc.eKind == ENV_OBJECT_KIND::STATIC)
			{
				if (nullptr == pEnvInstanceController)
				{
					Rollback(CreatedObjects);
					return E_FAIL;
				}

				CEnvObject_Static* pStaticObject = dynamic_cast<CEnvObject_Static*>(pCreatedObject);
				if (nullptr == pStaticObject)
				{
					Rollback(CreatedObjects);
					return E_FAIL;
				}

				pStaticObject->Set_InstanceController(pEnvInstanceController);
			}
		}

		}

		if (Options.bSpawnAddedEnv || Options.bSpawnAddedLevelDesign)
		{
			for (const MAP_ADD_OBJECT& Added : Package.AddedObjectDescs)
			{
				const ADDED_MAP_OBJECT_FAMILY eFamily = Resolve_AddedMapObjectFamily(Added.strPrototypeTag);

				CGameObject* pCreatedObject = nullptr;
				ENV_OBJECT_DESC AddedEnvRuntimeDesc{};
				LD_OBJECT_ENTRY AddedLevelDesignRuntimeEntry{};
				void* pArg = nullptr;
				_uint iPrototypeLevel = {};
				_uint iPlaceLevel = {};

				if (ADDED_MAP_OBJECT_FAMILY::ENV == eFamily)
				{
					if (!Options.bSpawnAddedEnv)
						continue;

					AddedEnvRuntimeDesc = Make_AddedEnvRuntimeDesc(Added);
					pArg = &AddedEnvRuntimeDesc;
					iPrototypeLevel = Levels.iObjectLevel;
					iPlaceLevel = Targets.EnvEffect.iPlaceLevel;
				}
				else if (ADDED_MAP_OBJECT_FAMILY::LEVELDESIGN == eFamily)
				{
					if (!Options.bSpawnAddedLevelDesign)
						continue;

					const LD_SPAWN_SPEC* pPlacementSpec = CLevelDesign_Registry::Find_PlacementSpec(Added.strPrototypeTag);

					if (nullptr != pPlacementSpec)
					{
						_float4 vPosition = { 0.f, 0.f, 0.f, 1.f };
						JsonUtils::Try_ReadFloat4Array(Added.jObject, "Transform.vPosition", &vPosition);

						const _float3 vPlacementPosition = { vPosition.x, vPosition.y, vPosition.z };

						if (!CLevelDesign_Registry::Make_DefaultDesc(
							Added.strPrototypeTag,
							Levels.iLevelDesignModelPrototypeLevel,
							Added.strObjectTag,
							vPlacementPosition,
							&AddedLevelDesignRuntimeEntry))
						{
							Rollback(CreatedObjects);
							return E_FAIL;
						}

						pArg = std::visit(
							[](auto& Desc) -> void*
							{
								return static_cast<void*>(&Desc);
							},
							AddedLevelDesignRuntimeEntry);

						iPrototypeLevel = Levels.iLevelDesignPrototypeLevel;
					}
					else
					{
						iPrototypeLevel = Levels.iObjectLevel;
					}

					iPlaceLevel = Levels.iLevelDesignObjectLevel;
				}
				else
				{
					Log_GameContentWarning(
						"Added map object spawn failed: unsupported prototype="
						+ WstrToStr(Added.strPrototypeTag));

					Rollback(CreatedObjects);
					return E_FAIL;
				}

				if (FAILED(m_pProxy->Add_GameObject_Return(
					&pCreatedObject,
					iPrototypeLevel,
					Added.strPrototypeTag.c_str(),
					iPlaceLevel,
					Added.strLayerTag.c_str(),
					Added.strObjectTag,
					pArg)))
				{
					Log_GameContentWarning(
						"Added map object spawn failed: prototype="
						+ WstrToStr(Added.strPrototypeTag));

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
				pOutReport->strStageName = Package.StageDesc.strStageName;
			}

			if (Options.bSpawnEnv)
			{
				pOutReport->iEnvDescriptorCount = static_cast<_uint>(Package.EnvObjectDescs.size());
				pOutReport->iEnvCreatedCount = iEnvCreatedCount;
				pOutReport->iEnvSkippedMissingModel = Package.iEnvSkippedMissingModel;
				pOutReport->iEnvSkippedCreateFailed = iEnvSkippedCreateFailed;
				pOutReport->iEnvJsonLoadedCount = static_cast<_uint>(Package.EnvJsonPaths.size());
			}
		}

	if (nullptr != ppOutStage && Options.bSpawnStage)
		*ppOutStage = pStage;

	if (nullptr != ppOutEnvInstanceController && Options.bSpawnEnv)
		*ppOutEnvInstanceController = pEnvInstanceController;

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

const MAP_SPAWN_ROUTE* CMap_Spawner::Resolve_EnvRoute(const MAP_SPAWN_TARGETS& Targets, ENV_OBJECT_KIND eKind) const
{
	switch (eKind)
	{
	case ENV_OBJECT_KIND::STATIC: return &Targets.EnvStatic;
	case ENV_OBJECT_KIND::INTERACT: return &Targets.EnvInteract;
	case ENV_OBJECT_KIND::EFFECT: return &Targets.EnvEffect;
	default: return nullptr;
	}
}

const _tchar* CMap_Spawner::Get_EnvObjectProtoTag(const ENV_OBJECT_DESC& Desc) const
{
	if (ENV_OBJECT_KIND::INTERACT == Desc.eKind
		&& ENV_INTERACT_TYPE::PHYSICS_PROP == Desc.eInteractType)
	{
		return CEnvInteract_KickProp::PROTOTYPE_TAG;
	}

	if (Desc.eKind != ENV_OBJECT_KIND::EFFECT)
		return Get_EnvObjectProtoTag(Desc.eKind);

	switch (Desc.tEffect.eEffectType)
	{
	case ENV_EFFECT_TYPE::TONE_MAPPING_AREA:
		return CEnvTrigger_RenderGlobals::PROTOTYPE_TAG;

	case ENV_EFFECT_TYPE::FIELD_EFFECT:
	case ENV_EFFECT_TYPE::FLOWER_WING:
		return CEnvVolume_Effect::PROTOTYPE_TAG;

	case ENV_EFFECT_TYPE::DECOR_PARTS_CULLING_AREA:
		return CEnvVolume_Culling::PROTOTYPE_TAG;

	case ENV_EFFECT_TYPE::GRASS_WIND:
		return CEnvTrigger_EventPublisher::PROTOTYPE_TAG;

	case ENV_EFFECT_TYPE::LOCAL_AREA_LIGHT:
		return CEnvVolume_Light::PROTOTYPE_TAG;

	case ENV_EFFECT_TYPE::SPOT_LIGHT:
		return CEnv_SpotLight::PROTOTYPE_TAG;

	default:
		return CEnvTrigger_Generic::PROTOTYPE_TAG;
	}
}

const _tchar* CMap_Spawner::Get_EnvObjectProtoTag(ENV_OBJECT_KIND eKind) const
{
	switch (eKind)
	{
	case ENV_OBJECT_KIND::STATIC: return CEnvObject_Static::PROTOTYPE_TAG;
	case ENV_OBJECT_KIND::INTERACT: return CEnvObject_Interact::PROTOTYPE_TAG;
	case ENV_OBJECT_KIND::EFFECT: return CEnvTrigger_Generic::PROTOTYPE_TAG;
	default: return nullptr;
	}
}

_wstring CMap_Spawner::Make_EnvObjectName(const ENV_OBJECT_DESC& Desc) const
{
	wstring strName = L"Env_" + Desc.wstrObjectName;
	if (0 != Desc.iUid)
		strName += L"_" + to_wstring(Desc.iUid);
	else if (!Desc.wstrEntryKey.empty())
		strName += L"_" + Desc.wstrEntryKey;
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
