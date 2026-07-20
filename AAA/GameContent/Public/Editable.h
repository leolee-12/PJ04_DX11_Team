#pragma once
#include "GameContent_Defines.h"

#include <variant>

NS_BEGIN(Engine)
class CModel;
struct MESH_LAYER_IDX;
NS_END

NS_BEGIN(Client)

enum class EDITABLE_OBJECT_KIND : _uint
{
	MAP_SECTION,
	ENV_OBJECT,
	LEVEL_DESIGN_OBJECT,
};

enum class EDITABLE_MODEL_KIND : _uint
{
	NONE,
	NONANIM,
	ANIM,
};

enum EDITABLE_CAPABILITY : _uint
{
	EDIT_CAP_RENDERABLE			= 1u << 0,
	EDIT_CAP_CULL_DISTANCE		= 1u << 1,
	EDIT_CAP_CULL_FRUSTUM		= 1u << 2,
	EDIT_CAP_COLLISION_MESH		= 1u << 3,
	EDIT_CAP_SHADOW				= 1u << 4,
	EDIT_CAP_MESH_LAYER			= 1u << 5,
	EDIT_CAP_DECAL				= 1u << 6,
	EDIT_CAP_ANIMATION			= 1u << 7,
};

struct EDITABLE_MODEL_SLOT
{
	_wstring strLabel;
	EDITABLE_MODEL_KIND eKind = { EDITABLE_MODEL_KIND::NONE };
	CModel* pModel = { nullptr };
	_uint iMeshCount = { 0u };
};

struct EDIT_OBJECT_POLICY
{
	_bool bRenderable = { true };
	_bool bUseCullDistance = { false };
	_bool bUseCullFrustum = { false };
	_bool bUseCollMesh = { false };
	_bool bUseShadow = { false };
};

struct EDIT_OBJECT_COMMON_OVERRIDE
{
	_uint iPolicyMask = { 0u };
	EDIT_OBJECT_POLICY Policy = {};

	_bool bHasWorldMatrix = { false };
	_float4x4 matWorld = {};

	_bool bHasPublishEvent = { false };
	_wstring strPublishEventTag;

	_bool bHasReceiveEvent = { false };
	_wstring strReceiveEventTag;
};

struct EDIT_MAPSECTION_OVERRIDE
{
};

struct EDIT_ENVOBJECT_OVERRIDE
{
	_bool bHasNearDistAlpha = { false };
	_bool bUseNearDistAlpha = { false };

	_bool bHasDecalAlpha = { false };
	_float fDecalAlpha = { 1.f };
};

struct EDIT_LEVELDESIGN_OVERRIDE
{
};

using EDIT_CLASS_OVERRIDE = variant<
	monostate,
	EDIT_MAPSECTION_OVERRIDE,
	EDIT_ENVOBJECT_OVERRIDE,
	EDIT_LEVELDESIGN_OVERRIDE>;

struct EDIT_OBJECT_OVERRIDE_DESC
{
	EDITABLE_OBJECT_KIND eKind = { EDITABLE_OBJECT_KIND::MAP_SECTION };
	_wstring strStableKey;

	EDIT_OBJECT_COMMON_OVERRIDE Common = {};
	EDIT_CLASS_OVERRIDE ClassOverride = monostate{};
};

struct EDITABLE_DESC
{
	EDITABLE_OBJECT_KIND eKind = { EDITABLE_OBJECT_KIND::MAP_SECTION };
	_wstring strStableKey;
	_uint iCapabilities = { 0u };
	EDIT_OBJECT_POLICY Policy = {};
	vector<EDITABLE_MODEL_SLOT> ModelSlots;
};

class CLIENT_DLL IEditable
{
public:
    virtual ~IEditable() = default;

	virtual _bool Get_EditDesc(EDITABLE_DESC* pOutDesc) const = 0;
	virtual HRESULT Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy) = 0;
	virtual HRESULT On_EditTransformChanged() = 0;

	virtual const MESH_LAYER_IDX* Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const = 0;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) = 0;
};

NS_END