#include "MeshLayer_Profile.h"

NS_BEGIN(Client)

_uint MeshLayerProfile::Resolve_MapPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass)
{
    const _int iPass = Layer.iPass;

    if (!Is_ValidMapPassValue(iPass))
        return iFallbackPass;

    return static_cast<_uint>(iPass);
}

_uint MeshLayerProfile::Resolve_WorldPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass)
{
    const auto ePass = static_cast<WORLD_PASS>(Layer.iPass);

    if (WORLD_PASS::DEFAULT == ePass)
        return iFallbackPass;

    if (!Is_ValidWorldPassValue(Layer.iPass))
        return iFallbackPass;

    return static_cast<_uint>(ePass);
}

SHADOW_ALPHA_SOURCE MeshLayerProfile::Resolve_WorldShadowAlphaSourceFromLayer(const MESH_LAYER_IDX& Layer)
{
    return Resolve_WorldShadowAlphaSource(static_cast<WORLD_PASS>(Layer.iPass));
}

NS_END