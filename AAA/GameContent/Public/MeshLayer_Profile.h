#pragma once
#include "GameContent_Defines.h"
#include "Shader_PassMeta.h"

NS_BEGIN(Client)

enum class MESH_LAYER_PROFILE : _uint
{
	MAP,

	WORLD_NONANIM,
	WORLD_ANIM,
	WORLD_INSTANCE,
};

enum class MESH_LAYER_RENDER_KIND : _uint
{
	MAIN,
	SHADOW,
	DECAL
};

namespace MeshLayerProfile
{
	_uint Resolve_MapPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass);
	_uint Resolve_WorldPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass);

	SHADOW_ALPHA_SOURCE Resolve_WorldShadowAlphaSourceFromLayer(const MESH_LAYER_IDX& Layer);
}

NS_END