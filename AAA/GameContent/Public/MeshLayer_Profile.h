#pragma once
#include "GameContent_Defines.h"
#include "Shader_PassMeta.h"

NS_BEGIN(Client)

enum class MESH_LAYER_PROFILE : _uint
{
	MAP,
	NONANIM_PBR,
	ENV_INSTANCE,
	LD_ANIM_PBR
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
	_uint Resolve_NonAnimPBRPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass);
	_uint Resolve_EnvInstancePass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass);

	ENV_SHADOW_ALPHA_SOURCE Resolve_EnvShadowAlphaSourceFromLayer(const MESH_LAYER_IDX& Layer);
}

NS_END