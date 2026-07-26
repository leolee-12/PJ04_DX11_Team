#include "LevelDesign_Registry.h"
#include "LevelDesign_MonsterCatalog.h"
#include "WaddleDee.h"

#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Boundary.h"
#include "LevelDesign_Rail.h"
#include "LevelDesign_FallBorder.h"
#include "LD_WaterArea.h"
#include "LD_LavaArea.h"
#include "LD_AudioArea.h"
#include "LD_LensFlare.h"

#include "LevelDesign_Breakable.h"
#include "LD_PopFlower.h"
#include "LevelDesign_Ladder.h"
#include "LD_ArrowBoard.h"
#include "LevelDesign_Point.h"
#include "LevelDesign_Bush.h"
#include "LevelDesign_Starblock.h"
#include "LevelDesign_Food.h"

#include "LD_DeformObject.h"
#include "LD_Stage1BossDemo.h"
#include "LD_SlopeBoardA.h"
#include "LD_SlopeBoardB.h"
#include "LD_SlopeBoardC.h"
#include "LD_DeformCarBreakWall.h"
#include "LD_GarageRadio.h"
#include "LD_CopyEssence.h"
#include "LD_MeteorGenerator.h"
#include "LD_BattleBoundary.h"
#include "LD_KirbyBed.h"
#include "LD_Frame.h"
#include "LD_CamPivot.h"


#include "Parsing_Utils.h"

#include <mutex>

NS_BEGIN(Client)

namespace
{
	std::once_flag g_LevelDesignRegistryInitOnce;
	unordered_map<_wstring, LD_SPAWN_SPEC> g_Specs;
	unordered_map<_wstring, _wstring> g_PlacementSpecKeys;
	LD_SPAWN_SPEC g_FallbackSpec = {};

	_bool Build_WaddleDeeDesc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
	{
		if (nullptr == pOutEntry || Spec.eCategory != LD_CATEGORY::META)
			return false;

		LD_PARSED_OBJECT Desc{};
		static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
		Desc.eCategory = Spec.eCategory;

		const _string strCharaPath = "Chara." + WstrToStr(CommonDesc.strObjectName);
		_wstring strVariation;
		const _bool bHasVariation =
			JsonUtils::Try_ReadString(jEntry, strCharaPath + ".Variation.VariationType", &strVariation)
			|| JsonUtils::Try_ReadString(jEntry, strCharaPath + ".Variation.Variation", &strVariation)
			|| JsonUtils::Try_ReadString(jEntry, strCharaPath + ".MainComponent.VariationType", &strVariation)
			|| JsonUtils::Try_ReadString(jEntry, "Variation", &strVariation);

		if (bHasVariation)
		{
			const _uint iSeed = JsonUtils::Equals_NoCase(CommonDesc.strObjectName.c_str(), L"ArenaSpectator")
				? CommonDesc.iUid
				: 0u;
			Desc.strAIVariation = CWaddleDee::Resolve_FixedAnim(strVariation, iSeed);
		}

		XMStoreFloat4(&Desc.vRight, XMVectorNegate(XMLoadFloat4(&Desc.vRight)));
		XMStoreFloat4(&Desc.vLook, XMVectorNegate(XMLoadFloat4(&Desc.vLook)));

		*pOutEntry = std::move(Desc);
		return true;
	}

	_wstring Make_Key(const _wstring& strValue)
	{
		_wstring Result = strValue;
		for (wchar_t& ch : Result)
			ch = static_cast<wchar_t>(towlower(ch));

		return Result;
	}

	CGameObject* Create_WaddleDeePrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CWaddleDee::Create(pDevice, pContext);
	}

	CGameObject* Create_BoundaryPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Boundary::Create(pDevice, pContext);
	}

	CGameObject* Create_UnsupportedPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Unsupported::Create(pDevice, pContext);
	}

	CGameObject* Create_RailPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CLevelDesign_Rail::Create(pDevice, pContext);
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
}

void CLevelDesign_Registry::Initialize()
{
	std::call_once(g_LevelDesignRegistryInitOnce, []()
		{
			g_Specs.clear();
			g_PlacementSpecKeys.clear();

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
			Register_NPCs();
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

	const _wstring strObjectKey = Make_Key(strObjectName);
	const auto [Iter, Inserted] = g_Specs.try_emplace(strObjectKey, SafeSpec);

	if (!Inserted)
	{
#ifdef _DEBUG
		const _wstring strMessage =
			L"[LevelDesign_Registry] duplicate object name: " + strObjectName + L"\n";
		OutputDebugStringW(strMessage.c_str());
#endif
		return false;
	}

	if (nullptr != SafeSpec.pMakeDefaultDesc)
	{
		const _wstring strPrototypeKey = Make_Key(SafeSpec.strPrototypeTag);
		const _bool bPlacementInserted =
			g_PlacementSpecKeys.try_emplace(strPrototypeKey, strObjectKey).second;

		if (!bPlacementInserted)
		{
			g_Specs.erase(Iter);

#ifdef _DEBUG
			const _wstring strMessage =
				L"[LevelDesign_Registry] duplicate placement prototype: "
				+ SafeSpec.strPrototypeTag + L"\n";
			OutputDebugStringW(strMessage.c_str());
#endif
			return false;
		}
	}

	return true;
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

const LD_SPAWN_SPEC* CLevelDesign_Registry::Find_PlacementSpec(const _wstring& strPrototypeTag)
{
	Initialize();

	if (strPrototypeTag.empty())
		return nullptr;

	const auto PlacementIter = g_PlacementSpecKeys.find(Make_Key(strPrototypeTag));
	if (PlacementIter == g_PlacementSpecKeys.end())
		return nullptr;

	const auto SpecIter = g_Specs.find(PlacementIter->second);
	if (SpecIter == g_Specs.end() || nullptr == SpecIter->second.pMakeDefaultDesc)
		return nullptr;

	return &SpecIter->second;
}

_bool CLevelDesign_Registry::Make_DefaultDesc(const _wstring& strPrototypeTag, _uint iModelProtoLevel,
	const _wstring& strObjectTag, const _float3& vPosition, LD_OBJECT_ENTRY* pOutEntry)
{
	if (nullptr == pOutEntry || strPrototypeTag.empty() || strObjectTag.empty())
		return false;

	const LD_SPAWN_SPEC* pSpec = Find_PlacementSpec(strPrototypeTag);
	if (nullptr == pSpec || nullptr == pSpec->pMakeDefaultDesc)
		return false;

	LD_OBJECT_DESC CommonDesc{};
	CommonDesc.wstrSourcePath = L"MapOverride";
	CommonDesc.strSourceFile = L"AddedMapObjects";
	CommonDesc.strSection = L"AddedMapObjects";
	CommonDesc.strEntryKey = strObjectTag;
	CommonDesc.strObjectName = pSpec->strObjectName;
	CommonDesc.strKind = L"MapOverride";
	CommonDesc.vPosition = { vPosition.x, vPosition.y, vPosition.z, 1.f };
	CommonDesc.vParsedPosition = vPosition;
	CommonDesc.eCategory = pSpec->eCategory;

	return pSpec->pMakeDefaultDesc(CommonDesc, iModelProtoLevel, *pSpec, pOutEntry);
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
		|| strLayerTag == L"Layer_LevelDesign_NPC"
		|| strLayerTag == L"Layer_LevelDesign_Gimmick"
		|| strLayerTag == L"Layer_LevelDesign_Unsupported";
}

void CLevelDesign_Registry::Register_Core()
{
	Register_Unsupported(L"StartPortal", LD_CATEGORY::UNSUPPORTED, L"Layer_LevelDesign_Unsupported");
	Register_Unsupported(L"DoorZone", LD_CATEGORY::DOOR, L"Layer_LevelDesign_Gimmick");

	LD_SPAWN_SPEC RailSpec{};
	RailSpec.strObjectName = CLevelDesign_Rail::OBJECT_NAME;
	RailSpec.strPrototypeTag = CLevelDesign_Rail::PROTOTYPE_TAG;
	RailSpec.strLayerTag = CLevelDesign_Rail::LAYER_TAG;
	RailSpec.eCategory = LD_CATEGORY::RAIL;
	RailSpec.pPrototypeFactory = &Create_RailPrototype;
	RailSpec.ModelRequirements =
	{
			{ CLevelDesign_Rail::COASTER_RAIL_MODEL_PROTO_TAG,
			"../../Resources/Map/Gimmick/NonAnim/CoasterRail/CoasterRail.ysh", MODEL::NONANIM, false }
	};
	Register(CLevelDesign_Rail::OBJECT_NAME, RailSpec);
}

void CLevelDesign_Registry::Register_Volumes()
{
	CLevelDesign_Boundary::Register_LevelDesignSpecs();
	CLevelDesign_FallBorder::Register_LevelDesignSpecs();
	CLD_WaterArea::Register_LevelDesignSpecs();
	CLD_LavaArea::Register_LevelDesignSpecs();
}

void CLevelDesign_Registry::Register_GuideAudio()
{
	Register_Unsupported(L"GuideMovieArea", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"SlideInfoArea", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"IntroductionDemo", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");

	CLD_AudioArea::Register_LevelDesignSpecs();
	CLD_LensFlare::Register_LevelDesignSpecs();
	CLD_CamPivot::Register_LevelDesignSpecs();
}

void CLevelDesign_Registry::Register_NPCs()
{
	const _tchar* ObjectNames[] =
	{
		  L"MerchantWaddleDee",
		  L"TalkWaddleDee",
		  L"VassalWaddleDee",
		  L"KnowledgeWaddleDee",
		  L"PharmacyClerk",
		  L"FoodShopClerk",
		  L"MgameFoodClerk",
		  L"MgameBallClerk",
		  L"DeliveryServiceClerk",
		  L"ArenaClerk",
		  L"ArenaSpectator"
	};

	for (const _tchar* pObjectName : ObjectNames)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = pObjectName;
		Spec.strPrototypeTag = CWaddleDee::PROTOTYPE_TAG;
		Spec.strLayerTag = L"Layer_LevelDesign_NPC";
		Spec.eCategory = LD_CATEGORY::META;
		Spec.pPrototypeFactory = &Create_WaddleDeePrototype;
		Spec.pBuildDesc = &Build_WaddleDeeDesc;
		Spec.bUseFactoryResourceLoader = true;

		Register(Spec.strObjectName, Spec);
	}
}

void CLevelDesign_Registry::Register_ItemsAndBreakables()
{
	CLevelDesign_Starblock::Register_LevelDesignSpecs();
	CLevelDesign_Breakable::Register_LevelDesignSpecs();
	CLevelDesign_Food::Register_LevelDesignSpecs();
	CLevelDesign_Point::Register_LevelDesignSpecs();
	CLevelDesign_Bush::Register_LevelDesignSpecs();
	CLD_PopFlower::Register_LevelDesignSpecs();
}

void CLevelDesign_Registry::Register_EnemiesAndGimmicks()
{
	CLevelDesign_MonsterCatalog::Register_LevelDesignSpecs();

	Register_Unsupported(L"ChainStarter", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"BlockChainInvisible", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");
	Register_Unsupported(L"TwinkleSwitch", LD_CATEGORY::GIMMICK, L"Layer_LevelDesign_Gimmick");

	CLevelDesign_Ladder::Register_LevelDesignSpecs();
	CLD_DeformObject::Register_LevelDesignSpecs();
	CLD_Stage1BossDemo::Register_LevelDesignSpecs();
	CLD_SlopeBoardA::Register_LevelDesignSpecs();
	CLD_SlopeBoardB::Register_LevelDesignSpecs();
	CLD_SlopeBoardC::Register_LevelDesignSpecs();
	CLD_DeformCarBreakWall::Register_LevelDesignSpecs();
	CLD_GarageRadio::Register_LevelDesignSpecs();
	CLD_ArrowBoard::Register_LevelDesignSpecs();
	CLD_CopyEssence::Register_LevelDesignSpecs();
	CLD_MeteorGenerator::Register_LevelDesignSpecs();
	CLD_BattleBoundary::Register_LevelDesignSpecs();
	CLD_KirbyBed::Register_LevelDesignSpecs();
	CLD_Frame::Register_LevelDesignSpecs();
}

NS_END