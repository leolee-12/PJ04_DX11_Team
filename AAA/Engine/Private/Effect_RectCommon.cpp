#include "Effect_RectCommon.h"

#include "Shader.h"
#include "VIBuffer_Rect.h"

namespace
{
    template<typename T>
    HRESULT BindValue(Engine::CShader* pShader, const _char* pName, const T& Value)
    {
        return pShader->Bind_RawValue(pName, &Value, sizeof(T));
    }

    void EvaluateSpriteFrame(
        _int iFrameX, _int iFrameY, _float fRatio,
        _float2& vOutUV, _float2& vOutSize)
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

HRESULT Engine::EffectRect::Bind_ShaderValues(
    CShader* pShader, const VALUES& Values, _float fRoll)
{
    if (FAILED(Bind_StaticShaderValues(pShader, Values)) ||
        FAILED(Bind_SpriteShaderValues(pShader, Values)) ||
        FAILED(Bind_Roll(pShader, fRoll)))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Bind_StaticShaderValues(
    CShader* pShader, const VALUES& Values)
{
    if (FAILED(BindValue(pShader, "g_bBillboard", Values.bBillboard)) ||
        FAILED(BindValue(pShader, "g_bSpriteAniTexture", Values.bSpriteAniTexture)) ||
        FAILED(BindValue(pShader, "g_bSpriteAniMask", Values.bSpriteAniMask)))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Bind_SpriteShaderValues(
    CShader* pShader, const VALUES& Values)
{
    if (FAILED(BindValue(pShader, "g_vSpriteAniTexUV", Values.vCurrentTexUV)) ||
        FAILED(BindValue(pShader, "g_vSpriteAniTexSize", Values.vCurrentTexSize)) ||
        FAILED(BindValue(pShader, "g_vSpriteAniMaskUV", Values.vCurrentMaskUV)) ||
        FAILED(BindValue(pShader, "g_vSpriteAniMaskSize", Values.vCurrentMaskSize)))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Bind_Roll(CShader* pShader, _float fRoll)
{
    return BindValue(pShader, "g_fRoll", fRoll);
}

HRESULT Engine::EffectRect::Bind_ParticleDrawValues(
    CShader* pShader, const _float4x4& WorldMatrix,
    _float fAlpha, const _float3& vColor)
{
    if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)) ||
        FAILED(BindValue(pShader, "g_fAlpha", fAlpha)) ||
        FAILED(BindValue(pShader, "g_vColor", vColor)))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectRect::Begin_AndRender(
    CShader* pShader, CVIBuffer_Rect* pBuffer, _int iPass)
{
    if (FAILED(pShader->Begin(iPass)) ||
        FAILED(pBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

void Engine::EffectRect::Update_SpriteAnimations(
    VALUES& Values, _float fRatio, _bool bEvaluateDisabled)
{
    if (Values.bSpriteAniTexture == true || bEvaluateDisabled == true)
    {
        EvaluateSpriteFrame(
            Values.iTexFrameX, Values.iTexFrameY,
            Values.bSpriteAniTexture == true ? fRatio : 0.f,
            Values.vCurrentTexUV, Values.vCurrentTexSize);
    }

    if (Values.bSpriteAniMask == true || bEvaluateDisabled == true)
    {
        EvaluateSpriteFrame(
            Values.iMaskFrameX, Values.iMaskFrameY,
            Values.bSpriteAniMask == true ? fRatio : 0.f,
            Values.vCurrentMaskUV, Values.vCurrentMaskSize);
    }
}

void Engine::EffectRect::Initialize_DefaultValues(VALUES& Values)
{
    Values.bBillboard = false;

    Values.bSpriteAniTexture = false;
    Values.iTexFrameX = 1;
    Values.iTexFrameY = 1;

    Values.bSpriteAniMask = false;
    Values.iMaskFrameX = 1;
    Values.iMaskFrameY = 1;
}

_int Engine::EffectRect::Normalize_SpriteTimeMode(_int iMode)
{
    Helper::IntClamp(iMode, SPRITE_TIME_EFFECT, SPRITE_TIME_END - 1);
    return iMode;
}
