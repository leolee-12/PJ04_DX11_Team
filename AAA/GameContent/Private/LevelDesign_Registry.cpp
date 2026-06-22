#include "LevelDesign_Registry.h"
#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Breakable.h"
#include "LevelDesign_Rail.h"
#include "LevelDesign_Ladder.h"
#include "LevelDesign_Food.h"
#include "LevelDesign_Point.h"
#include "LevelDesign_Bush.h"

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

	CGameObject* Create_UnsupportedPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Unsupported::Create(pDevice, pContext);
	}

	CGameObject* Create_RailPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Rail::Create(pDevice, pContext);
	}

	CGameObject* Create_LadderPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Ladder::Create(pDevice, pContext);
	}

	CGameObject* Create_FoodPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Food::Create(pDevice, pContext);
	}

	CGameObject* Create_PointPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Point::Create(pDevice, pContext);
	}

	CGameObject* Create_BushPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Bush::Create(pDevice, pContext);
	}

	_bool Build_ParsedObjectDesc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
	{
		UNREFERENCED_PARAMETER(jEntry);

		if (nullptr == pOutEntry)
			return false;

		LD_PARSED_OBJECT Desc{};
		static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
		Desc.eCategory = Spec.eCategory;

		*pOutEntry = Desc;
		return true;
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
		Spec.pPrototypeFactory = &Create_UnsupportedPrototype;

		CLevelDesign_Registry::Register(strObjectName, Spec);
	}

	void Append_AllFoodModelRequirements(vector<LD_MODEL_REQUIREMENT>* pOutRequirements)
	{
		if (nullptr == pOutRequirements)
			return;

		struct FOOD_MODEL_RESOURCE
		{
			const _tchar* pPrototypeTag;
			const _char* pFilePath;
		};

		static const FOOD_MODEL_RESOURCE Catalog[] =
		{
				{ L"Proto_Component_Model_Food_Babybottle", "../../Resources/Map/Gimmick/NonAnim/Food/Babybottle.ysh" },
				{ L"Proto_Component_Model_Food_Banana", "../../Resources/Map/Gimmick/NonAnim/Food/Banana.ysh" },
				{ L"Proto_Component_Model_Food_BreadB", "../../Resources/Map/Gimmick/NonAnim/Food/BreadB.ysh" },
				{ L"Proto_Component_Model_Food_Cake", "../../Resources/Map/Gimmick/NonAnim/Food/Cake.ysh" },
				{ L"Proto_Component_Model_Food_CanJuice_FruitGreen", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuice_FruitGreen.ysh" },
				{ L"Proto_Component_Model_Food_CanJuice_FruitOrange", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuice_FruitOrange.ysh" },
				{ L"Proto_Component_Model_Food_CanJuice_FruitPurple", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuice_FruitPurple.ysh" },
				{ L"Proto_Component_Model_Food_CanJuice_FruitRed", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuice_FruitRed.ysh" },
				{ L"Proto_Component_Model_Food_CanJuice_FruitYellow", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuice_FruitYellow.ysh" },
				{ L"Proto_Component_Model_Food_CanJuice_TopL", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuice_TopL.ysh" },
				{ L"Proto_Component_Model_Food_CanJuiceGear", "../../Resources/Map/Gimmick/NonAnim/Food/CanJuiceGear.ysh" },
				{ L"Proto_Component_Model_Food_Canned", "../../Resources/Map/Gimmick/NonAnim/Food/Canned.ysh" },
				{ L"Proto_Component_Model_Food_Carrot", "../../Resources/Map/Gimmick/NonAnim/Food/Carrot.ysh" },
				{ L"Proto_Component_Model_Food_Cherry", "../../Resources/Map/Gimmick/NonAnim/Food/Cherry.ysh" },
				{ L"Proto_Component_Model_Food_Chocolate", "../../Resources/Map/Gimmick/NonAnim/Food/Chocolate.ysh" },
				{ L"Proto_Component_Model_Food_Coffee", "../../Resources/Map/Gimmick/NonAnim/Food/Coffee.ysh" },
				{ L"Proto_Component_Model_Food_Corn", "../../Resources/Map/Gimmick/NonAnim/Food/Corn.ysh" },
				{ L"Proto_Component_Model_Food_CupJuiceMall", "../../Resources/Map/Gimmick/NonAnim/Food/CupJuiceMall.ysh" },
				{ L"Proto_Component_Model_Food_CupJuicePark", "../../Resources/Map/Gimmick/NonAnim/Food/CupJuicePark.ysh" },
				{ L"Proto_Component_Model_Food_Doughnut", "../../Resources/Map/Gimmick/NonAnim/Food/Doughnut.ysh" },
				{ L"Proto_Component_Model_Food_EnergyDrink", "../../Resources/Map/Gimmick/NonAnim/Food/EnergyDrink.ysh" },
				{ L"Proto_Component_Model_Food_Friedegg", "../../Resources/Map/Gimmick/NonAnim/Food/Friedegg.ysh" },
				{ L"Proto_Component_Model_Food_Greenpepper", "../../Resources/Map/Gimmick/NonAnim/Food/Greenpepper.ysh" },
				{ L"Proto_Component_Model_Food_Hamburger", "../../Resources/Map/Gimmick/NonAnim/Food/Hamburger.ysh" },
				{ L"Proto_Component_Model_Food_Hotdog", "../../Resources/Map/Gimmick/NonAnim/Food/Hotdog.ysh" },
				{ L"Proto_Component_Model_Food_IceCandy", "../../Resources/Map/Gimmick/NonAnim/Food/IceCandy.ysh" },
				{ L"Proto_Component_Model_Food_IceCream", "../../Resources/Map/Gimmick/NonAnim/Food/IceCream.ysh" },
				{ L"Proto_Component_Model_Food_InvincibleCandy", "../../Resources/Map/Gimmick/NonAnim/Food/InvincibleCandy.ysh" },
				{ L"Proto_Component_Model_Food_KirbyCarDessert", "../../Resources/Map/Gimmick/NonAnim/Food/KirbyCarDessert.ysh" },
				{ L"Proto_Component_Model_Food_KirbyHamburger", "../../Resources/Map/Gimmick/NonAnim/Food/KirbyHamburger.ysh" },
				{ L"Proto_Component_Model_Food_Makaron", "../../Resources/Map/Gimmick/NonAnim/Food/Makaron.ysh" },
				{ L"Proto_Component_Model_Food_MaxTomato", "../../Resources/Map/Gimmick/NonAnim/Food/MaxTomato.ysh" },
				{ L"Proto_Component_Model_Food_Meat", "../../Resources/Map/Gimmick/NonAnim/Food/Meat.ysh" },
				{ L"Proto_Component_Model_Food_Melon", "../../Resources/Map/Gimmick/NonAnim/Food/Melon.ysh" },
				{ L"Proto_Component_Model_Food_MelonSoda", "../../Resources/Map/Gimmick/NonAnim/Food/MelonSoda.ysh" },
				{ L"Proto_Component_Model_Food_Mikan", "../../Resources/Map/Gimmick/NonAnim/Food/Mikan.ysh" },
				{ L"Proto_Component_Model_Food_MilkPack", "../../Resources/Map/Gimmick/NonAnim/Food/MilkPack.ysh" },
				{ L"Proto_Component_Model_Food_Omelet", "../../Resources/Map/Gimmick/NonAnim/Food/Omelet.ysh" },
				{ L"Proto_Component_Model_Food_Onigiri", "../../Resources/Map/Gimmick/NonAnim/Food/Onigiri.ysh" },
				{ L"Proto_Component_Model_Food_Popcorn", "../../Resources/Map/Gimmick/NonAnim/Food/Popcorn.ysh" },
				{ L"Proto_Component_Model_Food_Potato", "../../Resources/Map/Gimmick/NonAnim/Food/Potato.ysh" },
				{ L"Proto_Component_Model_Food_Pudding", "../../Resources/Map/Gimmick/NonAnim/Food/Pudding.ysh" },
				{ L"Proto_Component_Model_Food_Pumpkin", "../../Resources/Map/Gimmick/NonAnim/Food/Pumpkin.ysh" },
				{ L"Proto_Component_Model_Food_RoastChicken", "../../Resources/Map/Gimmick/NonAnim/Food/RoastChicken.ysh" },
				{ L"Proto_Component_Model_Food_SoftCream", "../../Resources/Map/Gimmick/NonAnim/Food/SoftCream.ysh" },
				{ L"Proto_Component_Model_Food_Steak", "../../Resources/Map/Gimmick/NonAnim/Food/Steak.ysh" },
				{ L"Proto_Component_Model_Food_Sushi", "../../Resources/Map/Gimmick/NonAnim/Food/Sushi.ysh" },
				{ L"Proto_Component_Model_Food_Takoyaki", "../../Resources/Map/Gimmick/NonAnim/Food/Takoyaki.ysh" },
				{ L"Proto_Component_Model_Food_Tomato", "../../Resources/Map/Gimmick/NonAnim/Food/Tomato.ysh" },
				{ L"Proto_Component_Model_Food_WaterMelon", "../../Resources/Map/Gimmick/NonAnim/Food/WaterMelon.ysh" }
		};

		pOutRequirements->clear();
		pOutRequirements->reserve(_countof(Catalog));

		for (const auto& Entry : Catalog)
		{
			pOutRequirements->push_back({ Entry.pPrototypeTag, Entry.pFilePath, ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM });
		}
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
			g_FallbackSpec.pPrototypeFactory = &Create_UnsupportedPrototype;
			g_FallbackSpec.pBuildDesc = &Build_ParsedObjectDesc;

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

	if (Spec.strPrototypeTag.empty() || Spec.strLayerTag.empty() || nullptr == Spec.pPrototypeFactory)
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

_bool CLevelDesign_Registry::Build_Entry(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, LD_OBJECT_ENTRY* pOutEntry)
{
	if (nullptr == pOutEntry)
		return false;

	const LD_SPAWN_SPEC* pSpec = Find(CommonDesc.strObjectName);
	if (nullptr == pSpec)
		pSpec = &Get_FallbackSpec();

	if (nullptr == pSpec->pBuildDesc)
		return false;

	return pSpec->pBuildDesc(CommonDesc, jEntry, *pSpec, pOutEntry);
}

_bool CLevelDesign_Registry::Resolve(const LD_OBJECT_ENTRY& Desc, LD_RESOLVED_SPAWN* pOutResolved)
{
	if (nullptr == pOutResolved)
		return false;

	*pOutResolved = {};
	pOutResolved->ObjectDesc = Desc;

	LD_OBJECT_DESC& ResolvedDesc = Get_LDObjectDesc(pOutResolved->ObjectDesc);

	const LD_SPAWN_SPEC* pSpec = Find(ResolvedDesc.strObjectName);
	if (nullptr == pSpec)
		pSpec = &Get_FallbackSpec();

	pOutResolved->Spec = *pSpec;
	pOutResolved->bFallback = (pSpec == &Get_FallbackSpec());
	ResolvedDesc.eCategory = pSpec->eCategory;

	if (pSpec->strPrototypeTag == CLevelDesign_Food::PROTOTYPE_TAG)
	{
		LD_FOOD_DESC* pFoodDesc = std::get_if<LD_FOOD_DESC>(&pOutResolved->ObjectDesc);

		if (nullptr == pFoodDesc || pSpec->wstrModelProtoTag.empty())
			return false;

		pFoodDesc->wstrModelProtoTag = pSpec->wstrModelProtoTag;
	}

	if (pSpec->strPrototypeTag == CLevelDesign_Point::PROTOTYPE_TAG)
	{
		LD_POINT_DESC* pPointDesc = std::get_if<LD_POINT_DESC>(&pOutResolved->ObjectDesc);

		if (nullptr == pPointDesc || pSpec->wstrModelProtoTag.empty())
			return false;

		pPointDesc->wstrModelProtoTag = pSpec->wstrModelProtoTag;
	}

	return !pOutResolved->Spec.strPrototypeTag.empty()
		&& !pOutResolved->Spec.strLayerTag.empty();
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
	Register_Unsupported(L"DoorZone", LD_CATEGORY::DOOR, L"Layer_LevelDesign_Gimmick");

	LD_SPAWN_SPEC RailSpec{};
	RailSpec.strObjectName = CLevelDesign_Rail::OBJECT_NAME;
	RailSpec.strPrototypeTag = CLevelDesign_Rail::PROTOTYPE_TAG;
	RailSpec.strLayerTag = CLevelDesign_Rail::LAYER_TAG;
	RailSpec.eCategory = LD_CATEGORY::RAIL;
	RailSpec.pPrototypeFactory = &Create_RailPrototype;
	Register(CLevelDesign_Rail::OBJECT_NAME, RailSpec);
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
	const vector<LD_MODEL_REQUIREMENT> PointModelRequirements =
	{
		  { CLevelDesign_Point::YELLOW_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopYellowL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM },
		  { CLevelDesign_Point::BLUE_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopBlueL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM },
		  { CLevelDesign_Point::GREEN_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopGreenL.ysh", ETOUI(LEVEL::GAMEPLAY),MODEL::NONANIM },
		  { CLevelDesign_Point::RED_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopRedL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM },
		  { CLevelDesign_Point::COIN_CLUSTER_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterSL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM },
		  { CLevelDesign_Point::COIN_CLUSTER_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterML.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM },
		  { CLevelDesign_Point::COIN_CLUSTER_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/NonAnim/Point/TopCoinClusterLL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM }
	};

	LD_SPAWN_SPEC PointSpec{};
	PointSpec.strPrototypeTag = CLevelDesign_Point::PROTOTYPE_TAG;
	PointSpec.strLayerTag = L"Layer_LevelDesign_Item";
	PointSpec.eCategory = LD_CATEGORY::ITEM;
	PointSpec.pPrototypeFactory = &Create_PointPrototype;

	PointSpec.strObjectName = L"PointStarYellow";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::YELLOW_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);

	PointSpec.strObjectName = L"PointStarBlue";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::BLUE_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);

	PointSpec.strObjectName = L"PointStarGreen";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::GREEN_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);

	PointSpec.strObjectName = L"PointStarRed";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::RED_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);

	PointSpec.strObjectName = L"CoinClusterS";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::COIN_CLUSTER_S_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);

	PointSpec.strObjectName = L"CoinClusterM";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::COIN_CLUSTER_M_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);
	
	PointSpec.strObjectName = L"CoinClusterL";
	PointSpec.wstrModelProtoTag = CLevelDesign_Point::COIN_CLUSTER_L_MODEL_PROTO_TAG;
	PointSpec.ModelRequirements = PointModelRequirements;
	Register(PointSpec.strObjectName, PointSpec);

	LD_SPAWN_SPEC FoodSpec{};
	FoodSpec.strPrototypeTag = CLevelDesign_Food::PROTOTYPE_TAG;
	FoodSpec.strLayerTag = L"Layer_LevelDesign_Item";
	FoodSpec.eCategory = LD_CATEGORY::FOOD;
	FoodSpec.pPrototypeFactory = &Create_FoodPrototype;
	Append_AllFoodModelRequirements(&FoodSpec.ModelRequirements);

	FoodSpec.strObjectName = L"EnergyDrink";
	FoodSpec.wstrModelProtoTag = CLevelDesign_Food::ENERGY_DRINK_MODEL_PROTO_TAG;
	Register(FoodSpec.strObjectName, FoodSpec);

	FoodSpec.strObjectName = L"DinnerRoastChicken";
	FoodSpec.wstrModelProtoTag = CLevelDesign_Food::DINNER_ROAST_CHICKEN_MODEL_PROTO_TAG;
	Register(FoodSpec.strObjectName, FoodSpec);

	FoodSpec.strObjectName = L"FruitCherry";
	FoodSpec.wstrModelProtoTag = CLevelDesign_Food::FRUIT_CHERRY_MODEL_PROTO_TAG;
	Register(FoodSpec.strObjectName, FoodSpec);

	FoodSpec.strObjectName = L"VegetableCarrot";
	FoodSpec.wstrModelProtoTag = CLevelDesign_Food::VEGETABLE_CARROT_MODEL_PROTO_TAG;
	Register(FoodSpec.strObjectName, FoodSpec);

	FoodSpec.strObjectName = L"SweetsDoughnut";
	FoodSpec.wstrModelProtoTag = CLevelDesign_Food::SWEETS_DOUGHNUT_MODEL_PROTO_TAG;
	Register(FoodSpec.strObjectName, FoodSpec);

	FoodSpec.strObjectName = L"FruitBanana";
	FoodSpec.wstrModelProtoTag = CLevelDesign_Food::FRUIT_BANANA_MODEL_PROTO_TAG;
	Register(FoodSpec.strObjectName, FoodSpec);

	CLevelDesign_Breakable::Register_LevelDesignSpecs();

	LD_SPAWN_SPEC BushSpec{};
	BushSpec.strPrototypeTag = CLevelDesign_Bush::PROTOTYPE_TAG;
	BushSpec.strLayerTag = L"Layer_LevelDesign_Gimmick";
	BushSpec.eCategory = LD_CATEGORY::FOLIAGE;
	BushSpec.pPrototypeFactory = &Create_BushPrototype;

	BushSpec.strObjectName = L"Bush2BasicS";
	BushSpec.ModelRequirements =
	{
		  { CLevelDesign_Bush::BUSH_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushS.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::ANIM },
		  { CLevelDesign_Bush::CUT_S_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutS.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM }
	};
	Register(BushSpec.strObjectName, BushSpec);

	BushSpec.strObjectName = L"Bush2BasicM";
	BushSpec.ModelRequirements =
	{
		  { CLevelDesign_Bush::BUSH_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushM.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::ANIM },
		  { CLevelDesign_Bush::CUT_M_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutM.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM }
	};
	Register(BushSpec.strObjectName, BushSpec);

	BushSpec.strObjectName = L"Bush2BasicL";
	BushSpec.ModelRequirements =
	{
		  { CLevelDesign_Bush::BUSH_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/BushL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::ANIM },
		  { CLevelDesign_Bush::CUT_L_MODEL_PROTO_TAG, "../../Resources/Map/Gimmick/Anim/Bush/CutL.ysh", ETOUI(LEVEL::GAMEPLAY), MODEL::NONANIM }
	};
	Register(BushSpec.strObjectName, BushSpec);

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

	LD_SPAWN_SPEC LadderSpec{};
	LadderSpec.strObjectName = L"Ladder";
	LadderSpec.strPrototypeTag = CLevelDesign_Ladder::PROTOTYPE_TAG;
	LadderSpec.strLayerTag = L"Layer_LevelDesign_Gimmick";
	LadderSpec.eCategory = LD_CATEGORY::GIMMICK;
	LadderSpec.pPrototypeFactory = &Create_LadderPrototype;
	LadderSpec.ModelRequirements =
	{
			{
					CLevelDesign_Ladder::BOT_MODEL_PROTO_TAG,
					"../../Resources/Map/Gimmick/NonAnim/Ladder/Ladder_Bottom.ysh",
					ETOUI(LEVEL::GAMEPLAY)
			},
			{
					CLevelDesign_Ladder::MID_MODEL_PROTO_TAG,
					"../../Resources/Map/Gimmick/NonAnim/Ladder/Ladder_Middle.ysh",
					ETOUI(LEVEL::GAMEPLAY)
			},
			{
					CLevelDesign_Ladder::TOP_MODEL_PROTO_TAG,
					"../../Resources/Map/Gimmick/NonAnim/Ladder/Ladder_Top.ysh",
					ETOUI(LEVEL::GAMEPLAY)
			}
	};
	Register(LadderSpec.strObjectName, LadderSpec);

	Register_Unsupported(L"ArrowBoard", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
}

NS_END