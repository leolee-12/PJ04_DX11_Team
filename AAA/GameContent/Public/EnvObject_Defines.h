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

enum class ENV_COLLIDER_KIND
{
	NONE,
	SIMPLE_SHAPE,
	MODEL_MESH,
	TRIGGER_ONLY,
	UNKNOWN,
	END
};

enum class ENV_SIMPLE_SHAPE
{
	NONE,
	BOX,
	SPHERE,
	CYLINDER,
	SLOPE,
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
	_bool   bInvalidCollision = { false };
	_bool   bInvisibleCollision = { false };
	_bool   bSlipFallCollision = { false };
	_bool   bUseObjCollisionReaction = { false };
	_bool   bNeedUpdateCollisionByAnim = { false };
	_bool   bOverrideCollisionAttr = { false };

	wstring strOverrideCollisionType;
	wstring strOverrideCollisionTypeInside;
	wstring strMapCollType;

	_float  fMapCollRadius = { 0.f };
	_float3 vSize = {};

	ENV_COLLIDER_KIND eColliderKind = { ENV_COLLIDER_KIND::NONE };
	ENV_SIMPLE_SHAPE  eSimpleShape = { ENV_SIMPLE_SHAPE::NONE };

	_bool   bHasCollMesh = { false };
	_bool   bCookCollMesh = { false };
	_bool   bUseCollMesh = { false };

	// Legacy transitional metadata. Do not use these for new collision policy decisions.
	_bool   bCatalogCollisionChecked = { false };
	_bool   bHasDecorCollisionApxbin = { false };
	_bool   bSourceInvalidCollision = { false };
	_bool   bSourceHasDecorCollisionApxbin = { false };
	wstring strDecorCollisionApxbinName;
	wstring strDecorCollisionBfresPath;
};

struct ENV_RENDER_DESC
{
	_bool   bHasShadow = { false };
	_bool   bUseShadow = { false };
	_bool   bShadowMappingCaster = { false }; // Legacy transitional.

	_bool   bUseCullDistance = { false };
	_bool	bUseCullFrustum = { true };
	_bool   bUseNearDistAlpha = { false };
	_float  fNearDistAlphaLengthRate = { 1.f };
	wstring strLayerName;
	_uint   iHideFlag = {};
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

	_wstring wstrSourceFile;
	_wstring wstrSection;
	_wstring wstrEntryKey;
	_wstring wstrObjectName;
	_wstring wstrComponentName;
	_wstring wstrModelProtoTag;
	_wstring wstrModelPath;
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

inline _bool EnvObject_IsSimpleShapeOnly(const ENV_OBJECT_DESC& Desc)
{
	return Desc.eSourceType == ENV_SOURCE_TYPE::TOY_DECOR
		&& Desc.tCollision.eColliderKind == ENV_COLLIDER_KIND::SIMPLE_SHAPE;
}

inline _bool EnvObject_NeedsModel(const ENV_OBJECT_DESC& Desc)
{
	if (Desc.eKind == ENV_OBJECT_KIND::EFFECT)
		return false;

	if (EnvObject_IsSimpleShapeOnly(Desc))
		return false;

	return Desc.eKind == ENV_OBJECT_KIND::STATIC
		|| Desc.eKind == ENV_OBJECT_KIND::INTERACT;
}

struct ENV_INSTANCE_DATA
{
	_float4x4 matWorld = {};
};

struct ENV_INSTANCE_KEY
{
	_uint iModelProtoLevel = { 0 };
	_wstring wstrModelProtoTag = {};
	RENDERID eRenderID = { RENDERID::NONBLEND };

	_bool operator==(const ENV_INSTANCE_KEY& rhs) const
	{
		return iModelProtoLevel == rhs.iModelProtoLevel
			&& wstrModelProtoTag == rhs.wstrModelProtoTag
			&& eRenderID == rhs.eRenderID;
	}
};

struct ENV_INSTANCE_KEY_HASH
{
	size_t operator()(const ENV_INSTANCE_KEY& key) const
	{
		size_t h0 = std::hash<_uint>{}(key.iModelProtoLevel);
		size_t h1 = std::hash<_wstring>{}(key.wstrModelProtoTag);
		size_t h2 = std::hash<_uint>{}(ETOUI(key.eRenderID));
		return h0 ^ (h1 << 1) ^ (h2 << 2);
	}
};

constexpr _uint INVALID_INDEX = static_cast<_uint>(-1);

struct ENV_INSTANCE_BATCH_HANDLE
{
	_uint iMainBatchIndex = INVALID_INDEX;
	_uint iShadowBatchIndex = INVALID_INDEX;
};

NS_END
