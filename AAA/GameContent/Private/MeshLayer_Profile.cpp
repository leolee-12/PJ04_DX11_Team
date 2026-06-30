#include "MeshLayer_Profile.h"

NS_BEGIN(Client)

_uint MeshLayerProfile::Resolve_MapPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass)
{
    const _int iPass = Layer.iPass;

    if (!Is_ValidMapPassValue(iPass))
        return iFallbackPass;

    return static_cast<_uint>(iPass);
}

_uint MeshLayerProfile::Resolve_NonAnimPBRPass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass)
{
    const ENV_SHADER_PASS_META* pMeta = Find_EnvShaderPassMeta(Layer.iPass);
    if (nullptr == pMeta)
        return iFallbackPass;

    if (ENV_PASS::DEFAULT == pMeta->ePass)
        return iFallbackPass;

    return pMeta->iNonAnimPass;
}

_uint MeshLayerProfile::Resolve_EnvInstancePass(const MESH_LAYER_IDX& Layer, _uint iFallbackPass)
{
    const ENV_SHADER_PASS_META* pMeta = Find_EnvShaderPassMeta(Layer.iPass);
    if (nullptr == pMeta)
        return iFallbackPass;

    if (ENV_PASS::DEFAULT == pMeta->ePass)
        return iFallbackPass;

    return static_cast<_uint>(pMeta->ePass);
}

ENV_SHADOW_ALPHA_SOURCE MeshLayerProfile::Resolve_EnvShadowAlphaSourceFromLayer(const MESH_LAYER_IDX& Layer)
{
    const ENV_SHADER_PASS_META* pMeta = Find_EnvShaderPassMeta(Layer.iPass);
    if (nullptr == pMeta)
        return ENV_SHADOW_ALPHA_SOURCE::NONE;

    return Resolve_EnvShadowAlphaSource(pMeta->ePass);
}

NS_END