#pragma once
#include "Map_Defines.h"

NS_BEGIN(Client)

struct MAP_GIMMICK_FRAGMENT
{
	const _char*	pFragmentName;
	_float3			vPivot;
};

struct MAP_GIMMICK_SCATTER
{
	_float	fGravity = { -15.f };
	_float	fDespawnFallDistance = { 30.f };
	_float	fAngleStep = { 0.73f };
	_float	fBaseSpeed = { 8.f };
	_float	fSpeedStep = { 2.4f };
	_float	fBaseUpSpeed = { 12.f };
	_float	fUpSpeedStep = { 1.8f };
	_float3	vAngularBase = { 1.15f, 1.60f, 1.45f };
	_float	fAngularStep = { 0.5f };
};

struct MAP_GIMMICK_SECTION_ENTRY
{
	const _tchar*	pStageName;
	const _tchar*	pSectionName;
	const _tchar*	pShellSectionName;
	const _tchar*	pModelProtoTag;
	const _tchar*	pModelPath;
	const _tchar*	pObjectTag;

	COLLISION_LAYER eTriggerLayer;

	// CEffect_Loader에 사전 등록된 ID만 사용한다.
	const _tchar*	pBreakEffectID;
	_float			fEffectHeightRatio;
	_float			fEffectFrontRatio;

	const _tchar*	pBreakSFX;
	_float			fBreakSFXVolume;
	_float			fShakeTrauma;

	MAP_GIMMICK_SCATTER			Scatter;
	const MAP_GIMMICK_FRAGMENT*	pFragments;
	_uint						iNumFragments;
};

struct MAP_GIMMICK_BREAK_EVENT
{
	const MAP_GIMMICK_SECTION_ENTRY* pEntry = { nullptr };
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage1Step2_Fragments[] =
{
	  { "GsDefault_10", { 147.14450f, 27.23368f, 1306.22900f } },
	  { "GsDefault_11", { 147.42530f, 28.35020f, 1310.04500f } },
	  { "GsDefault_12", { 148.33050f, 26.39541f, 1307.77400f } },
	  { "GsDefault_13", { 148.99160f, 25.69413f, 1301.43300f } },
	  { "GsDefault_14", { 142.27910f, 26.67553f, 1302.79100f } },
	  { "GsDefault_15", { 143.78630f, 25.83631f, 1303.03300f } },
	  { "GsDefault_16", { 148.54220f, 26.77026f, 1302.02500f } },
	  { "GsDefault_17", { 149.38960f, 24.72021f, 1302.92300f } },
	  { "GsDefault_18", { 148.40590f, 28.09056f, 1303.71700f } },
	  { "GsDefault_19", { 143.31180f, 27.06821f, 1306.50100f } },
	  { "GsDefault_20", { 142.75370f, 28.40662f, 1303.28600f } },
	  { "GsDefault_21", { 144.59510f, 25.78181f, 1307.17300f } },
	  { "GsDefault_22", { 147.40860f, 26.11647f, 1308.86000f } },
	  { "GsDefault_23", { 140.99720f, 25.34992f, 1305.69600f } },
	  { "GsDefault_24", { 140.85890f, 27.90112f, 1310.78600f } },
	  { "GsDefault_25", { 141.34510f, 25.55193f, 1310.50000f } },
	  { "GsDefault_26", { 145.46370f, 26.66180f, 1310.14300f } },
};

inline constexpr MAP_GIMMICK_SECTION_ENTRY g_MapGimmickSections[] =
{
	  {
			  L"Stage1-2_MapStage",
			  L"GsDefault_4",
			  L"GsDefault_2",
			  L"Prototype_Component_Model_MapGimmickSection_Stage1-2_GsDefault_4",
			  L"../../Resources/Map/Stage1-2/Section/GsDefault_4.ysh",
			  L"MapGimmickSection_Stage1-2_GsDefault_4",

			  COLLISION_LAYER::CAR_BOOST,

			  L"Split_Stone_Ultra",
			  1.f,
			  -0.5f,

			  L"GimmickWallStake_Strike.wav",
			  0.6f,
			  1.f,

			  {},
			  g_Stage1Step2_Fragments,
			  static_cast<_uint>(_countof(g_Stage1Step2_Fragments)),
	  },
};

template <typename Fn>
inline void For_Each_MapGimmickEntry(const _wstring& strStageName, Fn&& fn)
{
	for (const MAP_GIMMICK_SECTION_ENTRY& Entry : g_MapGimmickSections)
	{
		if (strStageName == Entry.pStageName)
			fn(Entry);
	}
}

inline void Append_MapGimmickSectionDescs(const _wstring& strStageName, _uint iModelProtoLevel, vector<MAP_SECTION_DESC>* pOutDescs)
{
	if (nullptr == pOutDescs)
		return;

	For_Each_MapGimmickEntry(strStageName,
		[&](const MAP_GIMMICK_SECTION_ENTRY& Entry)
		{
			MAP_SECTION_DESC Desc{};
			Desc.strSectionName = Entry.pSectionName;
			Desc.wstrModelProtoTag = Entry.pModelProtoTag;
			Desc.wstrModelPath = Entry.pModelPath;
			Desc.iModelProtoLevel = iModelProtoLevel;
			pOutDescs->push_back(Desc);
		});
}

NS_END