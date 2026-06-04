#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

enum class ENV_OBJECT_KIND
{
	STATIC,
	INTERACT,
	EFFECT,
	UNKNOWN,
	END
};

enum class ENV_SOURCE_TYPE
{
	DECOR_DECOR,
	TOY_DECOR,
	TOY_OBJ,
	DECOR_OBJ,
	UNKNOWN,
	END
};

enum class ENV_EFFECT_TYPE
{
	NONE,
	LOCAL_AREA_LIGHT,
	TONE_MAPPING_AREA,
	DECOR_PARTS_CULLING_AREA,
	GRASS_WIND,
	FIELD_EFFECT,
	FLOWER_WING,
	SPOT_LIGHT,
	UNKNOWN,
	END
};

enum class ENV_INTERACT_TYPE
{
	NONE,
	HIT_REACT,
	BREAKABLE,
	PUSHABLE,
	COLLECTABLE,
	PHYSICS_PROP,
	ANIMATED_PROP,
	SMALL_CREATURE,
	UNKNOWN,
	END
};

struct ENV_BOUNDS_DESC
{
	_bool	bUseCullBounds = { false };
	_bool	bUseCustomBounds = { false };
	_float3 vCullBoundsCenter = {};
	_float3 vCullBoundsSize = {};
	_float3 vCustomBoundsCenter = {};
	_float3 vCustomBoundsSize = {};
};

struct ENV_COLLISION_DESC
{
	_bool	bInvalidCollision = { false };
	_bool	bInvisibleCollision = { false };
	_bool	bSlipFallCollision = { false };
	_bool	bUseObjCollisionReaction = { false };
	_bool	bNeedUpdateCollisionByAnim = { false };
	_bool	bOverrideCollisionAttr = { false };

	wstring	strOverrideCollisionType;
	wstring	strOverrideCollisionTypeInside;
	wstring	strMapCollType;

	_float	fMapCollRadius = { 0.f };
	_float3 vSize = {};
};

struct ENV_RENDER_DESC
{
	_bool	bShadowMappingCaster = { true };
	_bool	bUseLodCulling = { false };
	_bool	bUseNearDistAlpha = { false };
	_float	fNearDistAlphaLengthRate = { 1.f };
	wstring	strLayerName;
	_uint	iHideFlag = {};
};

struct ENV_EFFECT_DESC
{
	ENV_EFFECT_TYPE eEffectType = { ENV_EFFECT_TYPE::NONE };

	_float3 vAreaCenter = {};
	_float3 vAreaSize = {};
	_float4 vAreaRot = {};
	_float3 vEmitPos = {};

	wstring strAreaLightName;
	wstring strActivationCondition;
	wstring strHideKind;

	_float	fExposureValue = { 0.f };
	_float	fTransitionSec = { 0.f };
	_float	fInTransitionSec = { 0.f };
	_float	fOutTransitionSec = { 0.f };

	_float3 vPosition = {};
	_float3 vDirection = {};
	_float4 vColor = {};
	_float	fIntensity = { 0.f };
	_float	fRange = { 0.f };
	_float	fAngle = { 0.f };
	_float	fDecayStartAngle = { 0.f };
	wstring strKind;
};

struct ENV_OBJECT_DESC : public CGameObject::GAMEOBJECT_DESC
{
	ENV_OBJECT_KIND	eKind = { ENV_OBJECT_KIND::UNKNOWN };
	ENV_SOURCE_TYPE eSourceType = { ENV_SOURCE_TYPE::UNKNOWN };
	ENV_INTERACT_TYPE eInteractType = { ENV_INTERACT_TYPE::NONE };

	wstring strSourceFile;
	wstring strSection;
	wstring strEntryKey;
	wstring strObjectName;
	wstring strComponentName;
	wstring strModelProtoTag;
	wstring strModelPath;
	_uint	iModelProtoLevel = {};

	_uint	iUid = {};
	_int	iGroupId = { -1 };
	_int	iTargetLandGroupIndex = { -1 };

	_bool	bHasWorldMatrix = { false };
	_float4x4 matWorld = {};
	_float3 vPosition = {};
	_float4 vRotation = { 0.f, 0.f, 0.f, 1.f };
	_float3 vScale = { 1.f, 1.f, 1.f };

	wstring strLevel;
	wstring strSizeType;
	wstring strRepopType;
	wstring strInitCarryoverSettingType;

	ENV_BOUNDS_DESC	tBounds;
	ENV_COLLISION_DESC tCollision;
	ENV_RENDER_DESC tRender;
	ENV_EFFECT_DESC tEffect;

	json jRawProperties;
};

NS_END
