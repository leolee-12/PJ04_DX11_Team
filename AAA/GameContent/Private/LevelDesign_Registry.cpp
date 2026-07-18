#include "LevelDesign_Registry.h"
#include "LevelDesign_MonsterCatalog.h"
#include "WaddleDee.h"

#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Boundary.h"
#include "LevelDesign_Rail.h"
#include "LevelDesign_FallBorder.h"
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

#include "Parsing_Utils.h"

#include <mutex>

NS_BEGIN(Client)

namespace
{
	std::once_flag g_LevelDesignRegistryInitOnce;
	unordered_map<_wstring, LD_SPAWN_SPEC> g_Specs;
	LD_SPAWN_SPEC g_FallbackSpec = {};

	_bool Build_WaddleDeeDesc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
	{
		if (nullptr == pOutEntry || Spec.eCategory != LD_CATEGORY::META)
			return false;

		LD_PARSED_OBJECT Desc{};
		static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
		Desc.eCategory = Spec.eCategory;

		if (JsonUtils::Equals_NoCase(CommonDesc.strObjectName.c_str(), L"TalkWaddleDee"))
		{
			_wstring strVariation;
			if (JsonUtils::Try_ReadString(jEntry, "Chara.TalkWaddleDee.Variation.VariationType", &strVariation))
				Desc.strAIVariation = CWaddleDee::Resolve_FixedAnim(strVariation);
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
	Register_Unsupported(L"WaterArea", LD_CATEGORY::VOLUME, L"Layer_LevelDesign_Volume");
}

void CLevelDesign_Registry::Register_GuideAudio()
{
	Register_Unsupported(L"GuideMovieArea", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"SlideInfoArea", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");
	Register_Unsupported(L"IntroductionDemo", LD_CATEGORY::GUIDE_AREA, L"Layer_LevelDesign_Guide");

	CLD_AudioArea::Register_LevelDesignSpecs();
	CLD_LensFlare::Register_LevelDesignSpecs();
}

void CLevelDesign_Registry::Register_NPCs()
{
	const _tchar* ObjectNames[] =
	{
			L"MerchantWaddleDee",
			L"TalkWaddleDee"
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
}

NS_END