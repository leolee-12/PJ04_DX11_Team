#include "Effect_RectCommon.h"

#include "Shader.h"
#include "VIBuffer_Rect.h"

namespace
{
    void EvaluateSpriteFrame(_int iFrameX, _int iFrameY, _float fRatio, _float2& vOutUV, _float2& vOutSize)
    {
        if (iFrameX < 1)
            iFrameX = 1;
        if (iFrameY < 1)
            iFrameY = 1;

        Helper::FloatClamp(fRatio, 0.f, 1.f);

        const _int iTotalCount = iFrameX * iFrameY;
        _int iFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);
        if (iFrameIndex >= iTotalCount)
            iFrameIndex = iTotalCount - 1;

        vOutSize.x = 1.f / static_cast<_float>(iFrameX);
        vOutSize.y = 1.f / static_cast<_float>(iFrameY);
        vOutUV.x = vOutSize.x * static_cast<_float>(iFrameIndex % iFrameX);
        vOutUV.y = vOutSize.y * static_cast<_float>(iFrameIndex / iFrameX);
    }
}

HRESULT Engine::EffectRect::Bind_ShaderValues(CShader* pShader, const VALUES& Values, _float fRoll)
{
    if (FAILED(Bind_StaticShaderValues(pShader, Values)) ||
        FAILED(Bind_SpriteShaderValues(pShader, Values)) ||
        FAILED(Bind_Roll(pShader, fRoll)))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Bind_StaticShaderValues(CShader* pShader, const VALUES& Values)
{
    if (FAILED(pShader->Bind_RawValue("g_bBillboard", &Values.bBillboard, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bTextureColorToAlpha", &Values.bTextureColorToAlpha, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseTextureUVEdgeFade", &Values.TextureEdgeFade.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_iTextureUVEdgeFadeAxis", &Values.TextureEdgeFade.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_fTextureUVEdgeFadeStartRange", &Values.TextureEdgeFade.fStartRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fTextureUVEdgeFadeEndRange", &Values.TextureEdgeFade.fEndRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fTextureUVEdgeFadePower", &Values.TextureEdgeFade.fPower, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bSpriteAniTexture", &Values.bSpriteAniTexture, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bSpriteAniMask", &Values.bSpriteAniMask, sizeof(_bool))))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Bind_SpriteShaderValues(CShader* pShader, const VALUES& Values)
{
    if (FAILED(pShader->Bind_RawValue("g_vSpriteAniTexUV", &Values.vCurrentTexUV, sizeof(_float2))) ||
        FAILED(pShader->Bind_RawValue("g_vSpriteAniTexSize", &Values.vCurrentTexSize, sizeof(_float2))) ||
        FAILED(pShader->Bind_RawValue("g_vSpriteAniMaskUV", &Values.vCurrentMaskUV, sizeof(_float2))) ||
        FAILED(pShader->Bind_RawValue("g_vSpriteAniMaskSize", &Values.vCurrentMaskSize, sizeof(_float2))))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Bind_Roll(CShader* pShader, _float fRoll)
{
    return pShader->Bind_RawValue("g_fRoll", &fRoll, sizeof(_float));
}

HRESULT Engine::EffectRect::Bind_GBufferOutput(CShader* pShader, _bool bUseGBufferOutput)
{
    return pShader->Bind_RawValue("g_bWriteEmissiveGBuffer", &bUseGBufferOutput, sizeof(_bool));
}

HRESULT Engine::EffectRect::Bind_ParticleDrawValues(CShader* pShader, const _float4x4& WorldMatrix, _float fAlpha, const _float3& vColor)
{
    if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)) ||
        FAILED(pShader->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_vColor", &vColor, sizeof(_float3))))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Begin_AndRender(CShader* pShader, CVIBuffer_Rect* pBuffer, _int iPass)
{
    if (FAILED(pShader->Begin(iPass)) || FAILED(pBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

void Engine::EffectRect::Update_SpriteAnimations(VALUES& Values, _float fRatio, _bool bEvaluateDisabled)
{
    if (Values.bSpriteAniTexture == true || bEvaluateDisabled == true)
    {
        EvaluateSpriteFrame
        (
            Values.iTexFrameX, Values.iTexFrameY,
            Values.bSpriteAniTexture == true ? fRatio : 0.f,
            Values.vCurrentTexUV, Values.vCurrentTexSize
        );
    }

    if (Values.bSpriteAniMask == true || bEvaluateDisabled == true)
    {
        EvaluateSpriteFrame
        (
            Values.iMaskFrameX, Values.iMaskFrameY,
            Values.bSpriteAniMask == true ? fRatio : 0.f,
            Values.vCurrentMaskUV, Values.vCurrentMaskSize
        );
    }
}

void Engine::EffectRect::Initialize_DefaultValues(VALUES& Values)
{
    Values.bBillboard = false;
    Values.bTextureColorToAlpha = false;
    Values.TextureEdgeFade.bUse = false;
    Values.TextureEdgeFade.iAxis = 0;
    Values.TextureEdgeFade.fStartRange = 0.1f;
    Values.TextureEdgeFade.fEndRange = 0.1f;
    Values.TextureEdgeFade.fPower = 1.f;

    Values.bSpriteAniTexture = false;
    Values.iTexFrameX = 1;
    Values.iTexFrameY = 1;

    Values.bSpriteAniMask = false;
    Values.iMaskFrameX = 1;
    Values.iMaskFrameY = 1;
}
