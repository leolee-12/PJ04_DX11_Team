#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CModel;
class CShader;

namespace EffectMesh
{
    struct TEXTURE_VALUES
    {
        _bool& bUse;
        _float2& vTiling;
        _float2& vOffset;
        _bool& bUVScroll;
        _float2& vUVScrollCount;
        _float2& vCurrentOffset;
    };

    struct CIRCLE_UV_VALUES
    {
        _bool& bUse;
        _float& fRatio;
        _float& fStartRatio;
        _float& fEndRatio;
        _float& fStartDegree;
        _bool& bClockwise;
    };

    struct LINEAR_UV_VALUES
    {
        _bool& bUse;
        _float& fRatio;
        _float& fStartRatio;
        _float& fEndRatio;
        _int& iAxis;
        _bool& bReverse;
    };

    struct VALUES
    {
        _bool& bTextureColorToAlpha;
        TEXTURE_VALUES Diffuse;
        _bool& bDiffuseColorToAlpha;
        _bool& bUseNormalTexture;
        _bool& bUseMRATexture;
        TEXTURE_VALUES Unknown;

        CIRCLE_UV_VALUES TextureCircle;
        CIRCLE_UV_VALUES MaskCircle;
        CIRCLE_UV_VALUES DiffuseCircle;
        CIRCLE_UV_VALUES UnknownCircle;

        LINEAR_UV_VALUES TextureLinear;
        LINEAR_UV_VALUES MaskLinear;
        LINEAR_UV_VALUES DiffuseLinear;
        LINEAR_UV_VALUES UnknownLinear;
    };

    ENGINE_DLL HRESULT Bind_ShaderValues(CShader* pShader, const VALUES& Values, _bool bBindUnknownBeforePBR = false);
    ENGINE_DLL HRESULT Bind_Materials(CModel* pModel, CShader* pShader, _uint iMeshIndex, const VALUES& Values);

    ENGINE_DLL void Update_UVAnimations(VALUES& Values, _float fRatio);
    ENGINE_DLL void Initialize_DefaultValues(VALUES& Values);
}

NS_END
