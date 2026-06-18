#include "LevelDesign_Registry.h"
#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Breakable.h"

#include <cwctype>
#include <mutex>

NS_BEGIN(Client)

namespace
{
	std::once_flag g_LevelDesignRegistryInitOnce;
	unordered_map<_wstring, LD_SPAWN_SPEC> g_Specs;
	LD_SPAWN_SPEC g_FallbackSpec = {};

	_wstring Make_Key(const _wstring& strValue)
	{
		_wstring Result = strValue;
		for (wchar_t& ch : Result)
			ch = static_cast<wchar_t>(towlower(ch));

		return Result;
	}

	void Register_Unsupported(const _wstring& strObjectName, LD_CATEGORY eCategory, const _tchar* pLayerTag)
	{
		if (nullptr == pLayerTag)
			return;

		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = strObjectName;
		Spec.strPrototypeTag = CLevelDesign_Unsupported::PROTOTYPE_TAG;
		Spec.strLayerTag = pLayerTag;
		Spec.eCategory = eCategory;

		CLevelDesign_Registry::Register(strObjectName, Spec);
	}
}

void CLevelDesign_Registry::Initialize()
{
	std::call_once(g_LevelDesignRegistryInitOnce, []()
		{
			g_Specs.clear();

			g_FallbackSpec = {};
			g_FallbackSpec.strObjectName = L"Unsupported";
			g_FallbackSpec.strPrototypeTag = CLevelDesign_Unsupported::PROTOTYPE_TAG;
			g_FallbackSpec.strLayerTag = L"Layer_LevelDesign_Unsupported";
			g_FallbackSpec.eCategory = LD_CATEGORY::UNSUPPORTED;

			Register_Core();
			Register_Volumes();
			Register_GuideAudio();
			Register_ItemsAndBreakables();
			Register_EnemiesAndGimmicks();
		});
}

_bool CLevelDesign_Registry::Register(const _wstring& strObjectName, const LD_SPAWN_SPEC& Spec)
{
	if (strObjectName.empty())
		return false;

	if (Spec.strPrototypeTag.empty() || Spec.strLayerTag.empty())
		return false;

	LD_SPAWN_SPEC SafeSpec = Spec;
	if (SafeSpec.strObjectName.empty())
		SafeSpec.strObjectName = strObjectName;

	const auto [Iter, Inserted] = g_Specs.try_emplace(Make_Key(strObjectName), SafeSpec);

#ifdef _DEBUG
	if (!Inserted)
	{
		const _wstring strMessage =
			L"[LevelDesign_Registry] duplicate object name: " + strObjectName + L"\n";
		OutputDebugStringW(strMessage.c_str());
	}
#endif

	return Inserted;
}

const LD_SPAWN_SPEC* CLevelDesign_Registry::Find(const _wstring& strObjectName)
{
	Initialize();

	if (strObjectName.empty())
		return nullptr;

	const auto Iter = g_Specs.find(Make_Key(strObjectName));
	if (Iter == g_Specs.end())
		return nullptr;

	return &Iter->second;
}

const LD_SPAWN_SPEC& CLevelDesign_Registry::Get_FallbackSpec()
{
	Initialize();
	return g_FallbackSpec;
}

_bool CLevelDesign_Registry::Is_LevelDesignLayer(const _wstring& strLayerTag)
{
	return strLayerTag == L"Layer_LevelDesign_Portal"
		|| strLayerTag == L"Layer_LevelDesign_Rail"
		|| strLayerTag == L"Layer_LevelDesign_Volume"
		|| strLayerTag == L"Layer_LevelDesign_Guide"
		|| strLayerTag == L"Layer_LevelDesign_Audio"
		|| strLayerTag == L"Layer_LevelDesign_Item"
		|| strLayerTag == L"Layer_LevelDesign_Enemy"
		|| strLayerTag == L"Layer_LevelDesign_Gimmick"
		|| strLayerTag == L"Layer_LevelDesign_Unsupported";
}

void CLevelDesign_Registry::Register_Core()
{
	Register_Unsupported(L"StartPortal", LD_CATEGORY::PORTAL, L"Layer_LevelDesign_Portal");
	Register_Unsupported(L"Rail", LD_CATEGORY::RAIL, L"Layer_LevelDesign_Rail");
	Register_Unsupported(L"DoorZone", LD_CATEGORY::DOOR, L"Layer_LevelDesign_Gimmick");
}

void CLevelDesign_Registry::Register_Volumes()
{
	Register_Unsupported(L"InvisibleCollision", LD_CATEGORY::VOLUME, L"Layer_LevelDesign_Volume");
	Register_Unsupported(L"InvisibleCollisionBox", LD_CATEGORY::VOLUME, L"Layer_LevelDesign_Volume");
	Register_Unsupported(L"FallBorder", LD_CATEGORY::VOLUME, L"Layer_LevelDesign_Volume");
	Register_Unsupported(L"WaterArea", LD_CATEGORY::VOLUME, L"Layer_LevelDesign_Volume");
}

void CLevelDesign_Registry::Register_GuideAudio()
{
	Register_Unsupported(L"GuideMovieArea", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"SlideInfoArea", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"LensFlare", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"IntroductionDemo", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");

	Register_Unsupported(L"AreaBgmRequestor", LD_CATEGORY::AUDIO_AREA, L"Layer_LevelDesign_Audio");
	Register_Unsupported(L"AreaSeJungle", LD_CATEGORY::AUDIO_AREA, L"Layer_LevelDesign_Audio");
	Register_Unsupported(L"AreaSeSeaWave", LD_CATEGORY::AUDIO_AREA, L"Layer_LevelDesign_Audio");
}

void CLevelDesign_Registry::Register_ItemsAndBreakables()
{
	Register_Unsupported(L"PointStarYellow", LD_CATEGORY::ITEM, L"Layer_LevelDesign_Item");
	Register_Unsupported(L"PointStarBlue", LD_CATEGORY::ITEM, L"Layer_LevelDesign_Item");
	Register_Unsupported(L"PointStarGreen", LD_CATEGORY::ITEM, L"Layer_LevelDesign_Item");

	Register_Unsupported(L"EnergyDrink", LD_CATEGORY::FOOD, L"Layer_LevelDesign_Item");
	Register_Unsupported(L"DinnerRoastChicken", LD_CATEGORY::FOOD, L"Layer_LevelDesign_Item");
	Register_Unsupported(L"FruitCherry", LD_CATEGORY::FOOD, L"Layer_LevelDesign_Item");
	Register_Unsupported(L"VegetableCarrot", LD_CATEGORY::FOOD, L"Layer_LevelDesign_Item");

	LD_SPAWN_SPEC BreakableSpec{};
	BreakableSpec.strPrototypeTag = CLevelDesign_Breakable::PROTOTYPE_TAG;
	BreakableSpec.strLayerTag = L"Layer_LevelDesign_Gimmick";
	BreakableSpec.eCategory = LD_CATEGORY::BREAKABLE;

	BreakableSpec.strObjectName = L"StarBlock";
	Register(L"StarBlock", BreakableSpec);

	BreakableSpec.strObjectName = L"StarBlockBig";
	Register(L"StarBlockBig", BreakableSpec);

	BreakableSpec.strObjectName = L"WoodBox";
	Register(L"WoodBox", BreakableSpec);

	Register_Unsupported(L"Bush2BasicS", LD_CATEGORY::FOLIAGE, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"Bush2BasicM", LD_CATEGORY::FOLIAGE, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"Bush2BasicL", LD_CATEGORY::FOLIAGE, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"PopFlower", LD_CATEGORY::FOLIAGE, L"Layer_LevelDesign_Gimmick");
}

void CLevelDesign_Registry::Register_EnemiesAndGimmicks()
{
	Register_Unsupported(L"Kabu", LD_CATEGORY::ENEMY, L"Layer_LevelDesign_Enemy");
	Register_Unsupported(L"Cappy", LD_CATEGORY::ENEMY, L"Layer_LevelDesign_Enemy");
	Register_Unsupported(L"BrontoBurt", LD_CATEGORY::ENEMY, L"Layer_LevelDesign_Enemy");
	Register_Unsupported(L"BladeKnight", LD_CATEGORY::ENEMY, L"Layer_LevelDesign_Enemy");
	Register_Unsupported(L"PoppyBrosJr", LD_CATEGORY::ENEMY, L"Layer_LevelDesign_Enemy");

	Register_Unsupported(L"ChainStarter", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"BlockChainInvisible", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"TwinkleSwitch", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"Ladder", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"ArrowBoard", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
}

NS_END