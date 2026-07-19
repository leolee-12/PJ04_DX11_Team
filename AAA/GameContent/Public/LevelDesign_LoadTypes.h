#pragma once
#include "LevelDesign_Defines.h"

#include "GameObject.h"

#include <variant>

NS_BEGIN(Client)

constexpr _uint LD_INVALID_ID = 0xffffffff;

struct LD_OBJECT_DESC : public CGameObject::GAMEOBJECT_DESC
{
	_wstring wstrSourcePath;
	_wstring strSourceFile;
	_wstring strSection;
	_wstring strEntryKey;

	_wstring strObjectName;
	_wstring strKind;

	// 호준 추가
	_wstring strAIVariation;
	_wstring strThrowLv;

	_uint iUid = 0;
	_wstring strPublishEventTag;
	_wstring strReceiveEventTag;

	_uint iTargetRailUid = 0;
	_uint iTargetRailNodeIndex = 0;
	_int  iTargetLandGroupIndex = -1;

	_float3 vParsedPosition = {};
	_float4 qParsedRotation = { 0.f, 0.f, 0.f, 1.f };
	_float3 vParsedScale = { 1.f, 1.f, 1.f };

	LD_CATEGORY eCategory = LD_CATEGORY::UNSUPPORTED;

	json jRaw;
};

struct LD_PORTAL_DESC
{
	_int iPortalNo = -1;
	_bool bUseRestartCheck = false;
	_float3 vRestartAreaOffs = {};
	_float4 qRestartAreaRot = { 0.f, 0.f, 0.f, 1.f };
	_float3 vRestartAreaSize = {};
};

#pragma region Breakable
struct LD_BREAKABLE_DESC : public LD_OBJECT_DESC
{
	_uint		iModelProtoLevel = {};
	MODEL		eModelType = MODEL::NONANIM;
	_wstring	wstrModelProtoTag;
};
#pragma endregion

#pragma region Rail
struct LD_RAIL_NODE_DESC
{
	_float3 vPosition = {};
	_float fBezierControlLength = 0.f;
};

enum class LD_RAIL_TYPE : _uint
{
	LINE,
	BEZIER,
	CIRCLE,
	UNKNOWN,
	END
};

struct LD_RAIL_DESC
{
	LD_RAIL_TYPE eType = LD_RAIL_TYPE::UNKNOWN;

	_uint iNodeCount = 0;
	_float3 vCenterPos = {};
	_float fRadius = 0.f;
	_float fBezierControlLength = 0.f;
	_bool bClockwise = false;
	_bool bClose = false;
	vector<LD_RAIL_NODE_DESC> Nodes;
};
#pragma endregion

#pragma region Ladder
struct LD_LADDER_DESC : public LD_OBJECT_DESC
{
	_uint iModelProtoLevel = {};
	_uint iLength = { 0u };
};
#pragma endregion

#pragma region EventObject
inline constexpr _uint LD_ANIM_SLOT_COUNT = 4;

struct LD_EVENTOBJECT_DESC : public LD_OBJECT_DESC
{
	_uint		iModelProtoLevel = {};
	MODEL		eModelType = MODEL::ANIM;
	_wstring	wstrModelProtoTag;
	_string		strAnimNames[LD_ANIM_SLOT_COUNT] = {};
	_bool		bUseCollMesh = true;
	_wstring	strAnimEventFile;
};
#pragma endregion

#pragma region DeformObject
struct LD_DEFORMOBJECT_DESC : public LD_EVENTOBJECT_DESC
{
	DEFORM_TYPE eDeformType = { DEFORM_TYPE::NONE };
	_float fInteractionRadius = { 0.f };
};
#pragma endregion

#pragma region Portal
#pragma endregion

#pragma region Food
struct LD_FOOD_DESC : public LD_OBJECT_DESC
{
	_uint		iModelProtoLevel = {};
	_wstring	wstrModelProtoTag;
	_float		fHealAmount = { 0.f };
};
#pragma endregion

#pragma region Point
struct LD_POINT_DESC : public LD_OBJECT_DESC
{
	_uint		iModelProtoLevel = {};
	_wstring	wstrModelProtoTag;
	_int		iValue = { 0 };
};
#pragma endregion

#pragma region Bush
struct LD_BUSH_DESC : public LD_OBJECT_DESC
{
	_uint iModelProtoLevel = {};
	_wstring wstrBasicProtoTag;
	_wstring wstrCutProtoTag;
	MODEL eBasicType = { MODEL::ANIM };
	MODEL eCutType = { MODEL::NONANIM };
	_bool bGenerateItem = { false };
};
#pragma endregion

#pragma region AudioArea
struct LD_AUDIO_AREA_DESC
{
	_uint iSoundId = 0;
	_wstring strVariationType;
	_wstring strShapeType;
	_float3 vAreaCenter = {};
	_bool bHasAreaCenter = false;
	_float3 vAreaSize = {};
	_uint iFadeInFrame = 0;
	_uint iInactivateFrame = 0;
};
#pragma endregion

#pragma region SurfaceArea
struct LD_SURFACE_AREA_DESC : public LD_OBJECT_DESC
{
	_uint           iModelProtoLevel = {};
	MODEL           eModelType = MODEL::NONANIM;
	_wstring        wstrModelProtoTag;
};
#pragma endregion

struct LD_VOLUME_DESC
{
	_float3 vAreaCenter = {};
	_float4 qAreaRot = { 0.f, 0.f, 0.f, 1.f };
	_float3 vAreaSize = {};
	_float fHeight = 0.f;
	vector<_float3> Points;
	_wstring strTargetKind;
};

struct LD_GUIDE_AREA_DESC
{
	_wstring strGuideMovieKind;
	_float3 vAreaCenter = {};
	_float4 qAreaRot = { 0.f, 0.f, 0.f, 1.f };
	_float3 vAreaSize = {};
	_uint iMinPlayFrame = 0;
	_uint iWaitFrame = 0;
	_uint iRelationActorUid = 0;
};

struct LD_EFFECT_AREA_DESC
{
	_float3 vAreaCenter = {};
	_float4 qAreaRot = { 0.f, 0.f, 0.f, 1.f };
	_float3 vAreaSize = {};

	_float3 vEffectPos = {};
	_float4 qEffectRot = { 0.f, 0.f, 0.f, 1.f };

	_bool bHasEffectPos = false;
	_bool bHasEffectRot = false;
};

struct LD_STEP_LINK_INFO
{
	_bool bIgnoreWarningAtOneWay = false;
	_bool bRoundTrip = false;
	_int iMoveStepValue = 0;
	json jRaw;
};

struct LD_PARSED_OBJECT : public LD_OBJECT_DESC
{
	LD_PORTAL_DESC Portal;
	LD_RAIL_DESC Rail;
	LD_VOLUME_DESC Volume;
	LD_GUIDE_AREA_DESC GuideArea;
	LD_AUDIO_AREA_DESC AudioArea;
	LD_EFFECT_AREA_DESC EffectArea;
};

using LD_OBJECT_ENTRY = variant<
	LD_PARSED_OBJECT
	, LD_BREAKABLE_DESC
	, LD_LADDER_DESC
	, LD_EVENTOBJECT_DESC
	, LD_FOOD_DESC
	, LD_POINT_DESC
	, LD_BUSH_DESC
	, LD_DEFORMOBJECT_DESC
	, LD_SURFACE_AREA_DESC
>;

inline const LD_OBJECT_DESC& Get_LDObjectDesc(const LD_OBJECT_ENTRY& Entry)
{
	const auto ToBaseDesc = [](const auto& Desc) -> const LD_OBJECT_DESC& { return static_cast<const LD_OBJECT_DESC&>(Desc); };

	return std::visit(ToBaseDesc, Entry);
}

inline LD_OBJECT_DESC& Get_LDObjectDesc(LD_OBJECT_ENTRY& Entry)
{
	const auto ToBaseDesc = [](auto& Desc) -> LD_OBJECT_DESC& { return static_cast<LD_OBJECT_DESC&>(Desc); };

	return std::visit(ToBaseDesc, Entry);
}

struct LD_PACKAGE
{
	_wstring wstrSourcePath;
	vector<LD_OBJECT_ENTRY> ObjectDescs;
	vector<LD_STEP_LINK_INFO> StepLinks;
};

using LD_OBJECT_CREATED_CALLBACK = void(*)(void* pContext, CGameObject* pObject,
	const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

struct LD_RUNTIME_LEVELS
{
	_uint iObjectLevel = {};
	_uint iPrototypeLevel = {};
	_uint iModelPrototypeLevel = {};
};

struct LD_SPAWN_REQUEST
{
	LD_RUNTIME_LEVELS Levels;
	_uint iPlaceLevel = {};

	LD_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr;
	void* pCallbackContext = nullptr;
};

struct LD_LOAD_RESULT
{
	_wstring wstrSourcePath;

	_uint iParsedObjectCount = {};
	_uint iSpawnCandidateCount = {};
	_uint iCreatedCount = {};
	_uint iFallbackSpecCount = {};
	_uint iSkippedCreateFailedCount = {};
};

NS_END