#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CShader;
class CVIBuffer_Rect;

namespace EffectRect
{
    struct VALUES               // Non-owning references to the Effect object's members.
    {
        _bool& bBillboard;

        _bool& bSpriteAniTexture;
        _int& iTexFrameX;
        _int& iTexFrameY;
        _float2& vCurrentTexUV;
        _float2& vCurrentTexSize;

        _bool& bSpriteAniMask;
        _int& iMaskFrameX;
        _int& iMaskFrameY;
        _float2& vCurrentMaskUV;
        _float2& vCurrentMaskSize;
    };

    ENGINE_DLL HRESULT Bind_ShaderValues(CShader* pShader, const VALUES& Values, _float fRoll);

    ENGINE_DLL HRESULT Bind_StaticShaderValues(CShader* pShader, const VALUES& Values);

    ENGINE_DLL HRESULT Bind_SpriteShaderValues(CShader* pShader, const VALUES& Values);

    ENGINE_DLL HRESULT Bind_Roll(CShader* pShader, _float fRoll);

    ENGINE_DLL HRESULT Bind_ParticleDrawValues(CShader* pShader, const _float4x4& WorldMatrix, _float fAlpha, const _float3& vColor);

    ENGINE_DLL HRESULT Begin_AndRender(CShader* pShader, CVIBuffer_Rect* pBuffer, _int iPass);

    ENGINE_DLL void Update_SpriteAnimations(VALUES& Values, _float fRatio, _bool bEvaluateDisabled = false);

    ENGINE_DLL void Initialize_DefaultValues(VALUES& Values);
}

NS_END
