#include "EnvInteract_PresetCatalog.h"
#include "Parsing_Utils.h"

NS_BEGIN(Client)

namespace
{
	struct ENV_INTERACT_CATALOG_ENTRY
	{
		const _tchar* pObjectName;
		ENV_INTERACT_TYPE eType;
		_uint iRewardChancePercent;
		_int iPointStarAmount;
	};

	static const ENV_INTERACT_CATALOG_ENTRY g_InteractCatalog[] =
	{
			{ L"InteractiveGsPabble01L", ENV_INTERACT_TYPE::PHYSICS_PROP, 20u, 1 },
			{ L"InteractiveGsPabble02L", ENV_INTERACT_TYPE::PHYSICS_PROP, 20u, 1 },
			{ L"InteractiveGsEmptyCan01L", ENV_INTERACT_TYPE::PHYSICS_PROP, 10u, 1 },
			{ L"TrashCan", ENV_INTERACT_TYPE::BREAKABLE, 30u, 1 },
			{ L"PuffFlower", ENV_INTERACT_TYPE::BLOOM_PROP, 20u, 1 }
	};

	ENV_INTERACT_PRESET Make_BehaviorPreset(ENV_INTERACT_TYPE eType)
	{
		ENV_INTERACT_PRESET Preset{};
		Preset.eType = eType;

		switch (eType)
		{
		case ENV_INTERACT_TYPE::PHYSICS_PROP:
			Preset.eShape = ENV_INTERACT_SHAPE::CAPSULE;
			Preset.bTouchByPlayerBody = true;
			break;

		case ENV_INTERACT_TYPE::BREAKABLE:
			Preset.eShape = ENV_INTERACT_SHAPE::BOX;
			Preset.bDamageable = true;
			break;

		case ENV_INTERACT_TYPE::BLOOM_PROP:
			Preset.eShape = ENV_INTERACT_SHAPE::SPHERE;
			Preset.bTouchByPlayerBody = true;
			Preset.fTriggerRadius = 0.9f;
			break;

		default:
			break;
		}

		return Preset;
	}

	const ENV_INTERACT_CATALOG_ENTRY* Find_InteractCatalog(const _wstring& wstrObjectName)
	{
		for (const ENV_INTERACT_CATALOG_ENTRY& Entry : g_InteractCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, wstrObjectName.c_str()))
				return &Entry;
		}

		return nullptr;
	}
}

_bool CEnvInteract_PresetCatalog::Try_Find(const _wstring& wstrObjectName, ENV_INTERACT_PRESET* pOutPreset)
{
	if (wstrObjectName.empty() || nullptr == pOutPreset)
		return false;

	const ENV_INTERACT_CATALOG_ENTRY* pCatalog = Find_InteractCatalog(wstrObjectName);
	if (nullptr == pCatalog)
		return false;

	*pOutPreset = Make_BehaviorPreset(pCatalog->eType);
	pOutPreset->iRewardChancePercent = pCatalog->iRewardChancePercent;
	pOutPreset->iPointStarAmount = pCatalog->iPointStarAmount;
	pOutPreset->bGrantReward = 0u < pCatalog->iRewardChancePercent && 0 < pCatalog->iPointStarAmount;

	return true;
}

NS_END