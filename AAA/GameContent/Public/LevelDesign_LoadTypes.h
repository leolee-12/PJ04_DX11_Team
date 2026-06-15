#pragma once
#include "LevelDesign_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

constexpr _uint LD_INVALID_ID = 0xffffffff;

struct LD_COMMON_DESC : public CGameObject::GAMEOBJECT_DESC
{
    _wstring strSourcePath;
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

struct LD_PARSED_OBJECT : public LD_COMMON_DESC
{
};

struct LD_BREAKABLE_OBJECT_DESC : public LD_COMMON_DESC
{
    LD_BREAKABLE_TYPE eBreakableType = LD_BREAKABLE_TYPE::UNKNOWN;
};

NS_END