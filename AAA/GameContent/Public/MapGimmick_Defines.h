#pragma once
#include "Map_Defines.h"

NS_BEGIN(Client)

struct MAP_GIMMICK_FRAGMENT
{
	const _char* pFragmentName;
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
	const _tchar* pStageName;
	const _tchar* pSectionName;
	const _tchar* pShellSectionName;
	const _tchar* pModelProtoTag;
	const _tchar* pModelPath;
	const _tchar* pObjectTag;

	COLLISION_LAYER eTriggerLayer;
	_float			fTriggerPadding;

	// CEffect_Loader에 사전 등록된 ID만 사용한다.
	const _tchar* pBreakEffectID;
	_float			fEffectHeightRatio;
	_float			fEffectFrontRatio;

	const _tchar* pBreakSFX;
	_float			fBreakSFXVolume;
	_float			fShakeTrauma;

	MAP_GIMMICK_SCATTER			Scatter;
	const MAP_GIMMICK_FRAGMENT* pFragments;
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
	{ "GsDefault_22", { 140.40865f, 26.11647f, 1308.86000f } },
	{ "GsDefault_23", { 140.99720f, 25.34992f, 1305.69600f } },
	{ "GsDefault_24", { 140.85890f, 27.90112f, 1310.78600f } },
	{ "GsDefault_25", { 141.34510f, 25.55193f, 1310.50000f } },
	{ "GsDefault_26", { 145.46370f, 26.66180f, 1310.14300f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step1_GsAllBuilding7_Fragments[] =
{
		{ "GsAllBuilding_12", { 26.76442f, 148.95001f, -199.96893f } },
		{ "GsAllBuilding_13", { 27.92406f, 148.68582f, -202.00000f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step1_GsAllBuilding8_Fragments[] =
{
		{ "GsAllBuilding_21", { 2.81899f, 148.95000f, -230.37640f } },
		{ "GsAllBuilding_22", { 2.43712f, 148.95000f, -214.79863f } },
		{ "GsAllBuilding_23", { 3.90418f, 149.02077f, -210.74374f } },
		{ "GsAllBuilding_24", { 6.04157f, 148.95001f, -209.07892f } },
		{ "GsAllBuilding_25", { 2.08219f, 148.95000f, -227.16605f } },
		{ "GsAllBuilding_26", { -2.30205f, 149.03148f, -227.49228f } },
		{ "GsAllBuilding_27", { 2.51905f, 148.95000f, -219.86093f } },
		{ "GsAllBuilding_28", { -6.67062f, 148.95001f, -226.45749f } },
		{ "GsAllBuilding_29", { -0.61679f, 149.01315f, -222.64931f } },
		{ "GsAllBuilding_30", { -0.62299f, 148.95000f, -233.97356f } },
		{ "GsAllBuilding_31", { 2.09967f, 148.95000f, -232.34680f } },
		{ "GsAllBuilding_32", { -4.96511f, 148.95000f, -231.13065f } },
		{ "GsAllBuilding_33", { -6.17627f, 148.95560f, -236.33820f } },
		{ "GsAllBuilding_34", { 3.81000f, 148.95000f, -226.07828f } },
		{ "GsAllBuilding_35", { -2.85982f, 149.07500f, -220.69391f } },
		{ "GsAllBuilding_36", { -4.53162f, 148.95000f, -211.98145f } },
		{ "GsAllBuilding_37", { -3.15631f, 148.95561f, -215.12845f } },
		{ "GsAllBuilding_38", { 2.54456f, 148.87918f, -208.69626f } },
		{ "GsAllBuilding_39", { 7.10361f, 148.95001f, -206.25165f } },
		{ "GsAllBuilding_40", { 6.12683f, 148.80038f, -208.71083f } },
		{ "GsAllBuilding_41", { -4.75940f, 148.95001f, -208.16968f } },
		{ "GsAllBuilding_42", { 9.57707f, 148.95001f, -206.39307f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step1_GsAllBuilding9_Fragments[] =
{
		{ "GsAllBuilding_46", { -6.02400f, 148.95001f, -226.46875f } },
		{ "GsAllBuilding_47", { -6.01271f, 148.94998f, -222.59729f } },
		{ "GsAllBuilding_48", { -6.05611f, 148.95560f, -216.99316f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step1_GsAllBuilding10_Fragments[] =
{
		{ "GsAllBuilding_52", { 34.30952f, 128.95000f, -165.88727f } },
		{ "GsAllBuilding_53", { 34.68355f, 128.95001f, -164.70564f } },
		{ "GsAllBuilding_54", { 31.36531f, 128.92729f, -161.18677f } },
		{ "GsAllBuilding_55", { 33.19772f, 128.95003f, -160.30539f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step2_GsAllBuilding9_Fragments[] =
{
			  { "GsAllBuilding_25", { 2.06822f, 18.24187f, -398.86078f } },
			  { "GsAllBuilding_26", { 1.34762f, 17.99475f, -394.22601f } },
			  { "GsAllBuilding_27", { 3.18708f, 19.56681f, -395.46527f } },
			  { "GsAllBuilding_28", { 3.82005f, 13.94273f, -394.93671f } },
			  { "GsAllBuilding_29", { 2.48953f, 14.60481f, -396.81567f } },
			  { "GsAllBuilding_30", { 0.38663f, 13.24096f, -395.08026f } },
			  { "GsAllBuilding_31", { 1.93333f, 12.74549f, -394.09656f } },
			  { "GsAllBuilding_32", { -0.23275f, 15.99444f, -395.05182f } },
			  { "GsAllBuilding_33", { -0.19802f, 13.25999f, -400.49170f } },
			  { "GsAllBuilding_34", { -0.42613f, 14.94378f, -400.89642f } },
			  { "GsAllBuilding_35", { 4.46077f, 12.88363f, -400.61823f } },
			  { "GsAllBuilding_36", { 1.41216f, 16.64662f, -401.01801f } },
			  { "GsAllBuilding_37", { 3.14788f, 21.55787f, -401.65009f } },
			  { "GsAllBuilding_38", { 4.96601f, 20.37517f, -398.02524f } },
			  { "GsAllBuilding_39", { 3.18939f, 20.81031f, -397.89578f } },
			  { "GsAllBuilding_40", { 3.13480f, 18.76262f, -401.59900f } },
			  { "GsAllBuilding_41", { -3.06994f, 17.54233f, -395.34149f } },
			  { "GsAllBuilding_42", { -0.38588f, 19.89637f, -394.92010f } },
			  { "GsAllBuilding_43", { -1.74891f, 19.92692f, -398.19635f } },
			  { "GsAllBuilding_44", { -0.09189f, 19.65149f, -398.94513f } },
			  { "GsAllBuilding_45", { 1.03024f, 20.64308f, -393.92273f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step2_GsAllBuilding10_Fragments[] =
{
			  { "GsAllBuilding_51", { -0.52190f, 14.44308f, -373.86581f } },
			  { "GsAllBuilding_52", { 1.67336f, 13.44731f, -373.69318f } },
			  { "GsAllBuilding_53", { 0.98153f, 13.09709f, -377.80792f } },
			  { "GsAllBuilding_54", { 0.55653f, 17.72029f, -377.93701f } },
			  { "GsAllBuilding_55", { 1.73266f, 20.26668f, -378.44626f } },
			  { "GsAllBuilding_56", { 3.08737f, 20.37229f, -375.74573f } },
			  { "GsAllBuilding_57", { 1.71894f, 17.59178f, -375.78113f } },
			  { "GsAllBuilding_58", { 0.35332f, 18.97992f, -378.28671f } },
			  { "GsAllBuilding_59", { 3.84861f, 15.27655f, -375.61423f } },
			  { "GsAllBuilding_60", { -0.81676f, 16.65607f, -374.47021f } },
			  { "GsAllBuilding_61", { 1.01416f, 14.72159f, -375.89148f } },
			  { "GsAllBuilding_62", { -0.00821f, 20.33022f, -373.85043f } },
			  { "GsAllBuilding_63", { 0.31252f, 20.60296f, -375.69269f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step2_GsAllBuilding11_Fragments[] =
{
			  { "GsAllBuilding_69", { -0.73498f, 14.38791f, -380.76941f } },
			  { "GsAllBuilding_70", { 1.73519f, 13.25847f, -380.76404f } },
			  { "GsAllBuilding_71", { 0.98153f, 13.32887f, -384.24347f } },
			  { "GsAllBuilding_72", { 0.58136f, 17.30216f, -384.37103f } },
			  { "GsAllBuilding_73", { 0.91325f, 21.02764f, -384.89105f } },
			  { "GsAllBuilding_74", { 3.30963f, 20.53133f, -382.58179f } },
			  { "GsAllBuilding_75", { 1.75601f, 17.96899f, -382.95947f } },
			  { "GsAllBuilding_76", { 3.91546f, 15.74820f, -382.90753f } },
			  { "GsAllBuilding_77", { -0.54783f, 16.78305f, -381.97589f } },
			  { "GsAllBuilding_78", { 1.92883f, 14.95259f, -383.12299f } },
			  { "GsAllBuilding_79", { -1.16048f, 20.34940f, -380.99536f } },
			  { "GsAllBuilding_80", { -0.13492f, 20.55221f, -382.49951f } },
};

inline constexpr MAP_GIMMICK_FRAGMENT g_Stage3Step2_GsAllBuilding12_Fragments[] =
{
			  { "GsAllBuilding_86", { -0.67036f, 14.42742f, -387.18707f } },
			  { "GsAllBuilding_87", { 1.71679f, 13.31621f, -387.17816f } },
			  { "GsAllBuilding_88", { 0.98153f, 13.31770f, -390.83228f } },
			  { "GsAllBuilding_89", { 0.28023f, 17.28687f, -390.87708f } },
			  { "GsAllBuilding_90", { 0.72243f, 21.12493f, -391.44321f } },
			  { "GsAllBuilding_91", { 3.25857f, 20.60145f, -389.11407f } },
			  { "GsAllBuilding_92", { 1.67929f, 17.95930f, -389.15363f } },
			  { "GsAllBuilding_93", { 3.89897f, 15.72703f, -389.08539f } },
			  { "GsAllBuilding_94", { -0.63748f, 16.76123f, -388.24994f } },
			  { "GsAllBuilding_95", { 1.78818f, 14.91216f, -389.54156f } },
			  { "GsAllBuilding_96", { -0.74048f, 20.35007f, -387.37585f } },
			  { "GsAllBuilding_97", { 0.05079f, 20.55854f, -388.96439f } },
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

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		0.5f,

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
	{
		L"Stage3-1_MapStage",
		L"Land_GsAllBuilding_7",
		L"Land_GsAllBuilding_1",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-1_Land_GsAllBuilding_7",
		L"../../Resources/Map/Stage3-1/Section/Land_GsAllBuilding_7.ysh",
		L"MapGimmickSection_Stage3-1_Land_GsAllBuilding_7",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		0.5f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step1_GsAllBuilding7_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step1_GsAllBuilding7_Fragments)),
	},
	{
		L"Stage3-1_MapStage",
		L"Land_GsAllBuilding_8",
		L"Land_GsAllBuilding_2",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-1_Land_GsAllBuilding_8",
		L"../../Resources/Map/Stage3-1/Section/Land_GsAllBuilding_8.ysh",
		L"MapGimmickSection_Stage3-1_Land_GsAllBuilding_8",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		0.5f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step1_GsAllBuilding8_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step1_GsAllBuilding8_Fragments)),
	},
	{
		L"Stage3-1_MapStage",
		L"Land_GsAllBuilding_9",
		L"Land_GsAllBuilding_3",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-1_Land_GsAllBuilding_9",
		L"../../Resources/Map/Stage3-1/Section/Land_GsAllBuilding_9.ysh",
		L"MapGimmickSection_Stage3-1_Land_GsAllBuilding_9",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		0.5f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step1_GsAllBuilding9_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step1_GsAllBuilding9_Fragments)),
	},
	{
		L"Stage3-1_MapStage",
		L"Land_GsAllBuilding_10",
		L"Land_GsAllBuilding_4",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-1_Land_GsAllBuilding_10",
		L"../../Resources/Map/Stage3-1/Section/Land_GsAllBuilding_10.ysh",
		L"MapGimmickSection_Stage3-1_Land_GsAllBuilding_10",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		0.5f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step1_GsAllBuilding10_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step1_GsAllBuilding10_Fragments)),
	},
	{
		L"Stage3-2_MapStage",
		L"Land_GsAllBuilding_9",
		L"Land_GsAllBuilding_2",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-2_Land_GsAllBuilding_9",
		L"../../Resources/Map/Stage3-2/Section/Land_GsAllBuilding_9.ysh",
		L"MapGimmickSection_Stage3-2_Land_GsAllBuilding_9",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		2.f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step2_GsAllBuilding9_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step2_GsAllBuilding9_Fragments)),
	},
	{
		L"Stage3-2_MapStage",
		L"Land_GsAllBuilding_10",
		L"Land_GsAllBuilding_3",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-2_Land_GsAllBuilding_10",
		L"../../Resources/Map/Stage3-2/Section/Land_GsAllBuilding_10.ysh",
		L"MapGimmickSection_Stage3-2_Land_GsAllBuilding_10",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		2.f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step2_GsAllBuilding10_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step2_GsAllBuilding10_Fragments)),
	},
	{
		L"Stage3-2_MapStage",
		L"Land_GsAllBuilding_11",
		L"Land_GsAllBuilding_4",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-2_Land_GsAllBuilding_11",
		L"../../Resources/Map/Stage3-2/Section/Land_GsAllBuilding_11.ysh",
		L"MapGimmickSection_Stage3-2_Land_GsAllBuilding_11",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		2.f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step2_GsAllBuilding11_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step2_GsAllBuilding11_Fragments)),
	},
	{
		L"Stage3-2_MapStage",
		L"Land_GsAllBuilding_12",
		L"Land_GsAllBuilding_5",
		L"Prototype_Component_Model_MapGimmickSection_Stage3-2_Land_GsAllBuilding_12",
		L"../../Resources/Map/Stage3-2/Section/Land_GsAllBuilding_12.ysh",
		L"MapGimmickSection_Stage3-2_Land_GsAllBuilding_12",

		COLLISION_LAYER::PLAYER_BREAKERABLE,
		2.f,

		L"Split_Stone_Ultra",
		1.f,
		-0.5f,

		L"GimmickWallStake_Strike.wav",
		0.6f,
		1.f,

		{},
		g_Stage3Step2_GsAllBuilding12_Fragments,
		static_cast<_uint>(_countof(g_Stage3Step2_GsAllBuilding12_Fragments)),
	},
};

inline _bool Is_MapGimmickSection(const _wstring& strStageName, const _wstring& strSectionName)
{
	for (const MAP_GIMMICK_SECTION_ENTRY& Entry : g_MapGimmickSections)
	{
		if (strStageName == Entry.pStageName && strSectionName == Entry.pSectionName)
			return true;
	}

	return false;
}

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