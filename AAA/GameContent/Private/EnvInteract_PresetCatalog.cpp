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
			{ L"TrashCanC", ENV_INTERACT_TYPE::BREAKABLE, 30u, 1 },
	};

	_bool Has_InteractivePrefix(const _wstring& wstrObjectName)
	{
		static constexpr const _tchar* INTERACTIVE_PREFIX = L"Interactive";
		static constexpr size_t INTERACTIVE_PREFIX_LENGTH = 11;

		return wstrObjectName.size() > INTERACTIVE_PREFIX_LENGTH
			&& 0 == _wcsnicmp(wstrObjectName.c_str(), INTERACTIVE_PREFIX, INTERACTIVE_PREFIX_LENGTH);
	}

	ENV_INTERACT_PRESET Make_BehaviorPreset(ENV_INTERACT_TYPE eType)
	{
		ENV_INTERACT_PRESET Preset{};
		Preset.eType = eType;

		switch (eType)
		{
		case ENV_INTERACT_TYPE::PHYSICS_PROP:
			Preset.eShape = ENV_INTERACT_SHAPE::CAPSULE;
			Preset.bTouchByPlayerBody = true;
			Preset.fKickPower = 10.f;           // 수평
			Preset.fUpImpulse = 12.f;           // 수직 → 약 51° 대각선, 정점 약 2.5유닛
			Preset.fMaxSpeed = 30.f;
			Preset.fBounceRestitution = 0.5f;   // 두 번째 뜀은 절반 높이
			Preset.fBounceFriction = 0.6f;      // 0.2는 수평이 죽어 제자리에 주저앉음
			break;	

		case ENV_INTERACT_TYPE::BREAKABLE:
			Preset.eShape = ENV_INTERACT_SHAPE::BOX;
			Preset.bDamageable = true;
			Preset.wstrBreakSoundKey = L"GimmickTrashCan_Break.wav";
			Preset.wstrBreakEffectKey = L"Split_Trash";
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
	{
		if (!Has_InteractivePrefix(wstrObjectName))
			return false;

		*pOutPreset = Make_BehaviorPreset(ENV_INTERACT_TYPE::PHYSICS_PROP);
		return true;
	}

	*pOutPreset = Make_BehaviorPreset(pCatalog->eType);
	pOutPreset->iRewardChancePercent = pCatalog->iRewardChancePercent;
	pOutPreset->iPointStarAmount = pCatalog->iPointStarAmount;
	pOutPreset->bGrantReward = 0u < pCatalog->iRewardChancePercent && 0 < pCatalog->iPointStarAmount;

	return true;
}

NS_END