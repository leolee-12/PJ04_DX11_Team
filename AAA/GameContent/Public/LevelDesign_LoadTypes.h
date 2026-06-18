#pragma once
#include "LevelDesign_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

constexpr _uint LD_INVALID_ID = 0xffffffff;

struct LD_COMMON_DESC : public CGameObject::GAMEOBJECT_DESC
{
    _wstring wstrSourcePath;
    _wstring strSourceFile;
    _wstring strSection;
    _wstring strEntryKey;

    _wstring strObjectName;
    _wstring strKind;

    _uint iUid = 0;
    _uint iEventReceiverId = LD_INVALID_ID;
    _uint iEventSenderId = LD_INVALID_ID;
    _uint iTargetRailUid = 0;
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

struct LD_RAIL_DESC
{
    _uint iNodeCount = 0;
    _float3 vCenterPos = {};
    _float fRadius = 0.f;
    _wstring strErpType;
    _bool bClockwise = false;
    _bool bClose = false;
    vector<_float3> Nodes;
};

struct LD_VOLUME_DESC
{
    LD_VOLUME_TYPE eVolumeType = LD_VOLUME_TYPE::UNKNOWN;
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

struct LD_AUDIO_AREA_DESC
{
    LD_AUDIO_AREA_TYPE eAudioAreaType = LD_AUDIO_AREA_TYPE::UNKNOWN;
    _uint iSoundId = 0;
    _wstring strVariationType;
    _wstring strShapeType;
    _float3 vAreaCenter = {};
    _float4 qAreaRot = { 0.f, 0.f, 0.f, 1.f };
    _float3 vAreaSize = {};
    _uint iFadeInFrame = 0;
    _uint iInactivateFrame = 0;
};

struct LD_STEP_LINK_INFO
{
    _bool bIgnoreWarningAtOneWay = false;
    _bool bRoundTrip = false;
    _int iMoveStepValue = 0;
    json jRaw;
};

struct LD_PARSED_OBJECT : public LD_COMMON_DESC
{
    LD_PORTAL_DESC Portal;
    LD_RAIL_DESC Rail;
    LD_VOLUME_DESC Volume;
    LD_GUIDE_AREA_DESC GuideArea;
    LD_AUDIO_AREA_DESC AudioArea;
};

struct LD_BREAKABLE_OBJECT_DESC : public LD_COMMON_DESC
{
    LD_BREAKABLE_TYPE   eType = LD_BREAKABLE_TYPE::UNKNOWN;
    _wstring            wstrModelProtoTag;
};

struct LD_PACKAGE
{
    _wstring wstrSourcePath;
    vector<LD_PARSED_OBJECT> ObjectDescs;
    vector<LD_STEP_LINK_INFO> StepLinks;
};

using LD_OBJECT_CREATED_CALLBACK = void(*)(void* pContext, CGameObject* pObject,
    const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

struct LD_RUNTIME_LEVELS
{
    _uint iObjectLevel = {};
    _uint iPrototypeLevel = {};
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