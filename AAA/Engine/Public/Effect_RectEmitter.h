#pragma once

#include "Effect_Emitter.h"

NS_BEGIN(Engine)

class CVIBuffer_Rect;
namespace EffectRect { struct VALUES; }

class ENGINE_DLL CEffect_RectEmitter abstract : public CEffect_Emitter
{
    GENERATED_BODY_ABSTRACT(CEffect_RectEmitter)

PROPERTY(_bool, m_bBillboard, L"Billboard", L"Effect");
PROPERTY(_bool, m_bTextureColorToAlpha, L"Color To Alpha_T", L"Texture Com");
PROPERTY(_bool, m_bUseParticleRoll, L"Use Particle Roll", L"Rect Animation");

// Sprite Animation Texture
PROPERTY(_bool, m_bSpriteAniTexture, L"Sprite Animation Texture", L"Sprite Animation");

PROPERTY(_int, m_iTexFrameX, L"Frame X_T", L"Sprite Animation");
PROPERTY(_int, m_iTexFrameY, L"Frame Y_T", L"Sprite Animation");

// Sprite Animation Mask
PROPERTY(_bool, m_bSpriteAniMask, L"Sprite Animation Mask", L"Sprite Animation");

PROPERTY(_int, m_iMaskFrameX, L"Frame X_M", L"Sprite Animation");
PROPERTY(_int, m_iMaskFrameY, L"Frame Y_M", L"Sprite Animation");

public:
    struct EFFECT_RECTEMITTER_DESC : public CEffect_Emitter::EFFECT_EMITTER_DESC
    {
        // Buffer
        _uint iVIBufferLevel{};
        _wstring wstrVIBufferTag;

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

protected:
    CEffect_RectEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_RectEmitter(const CEffect_RectEmitter& Prototype);
    virtual ~CEffect_RectEmitter() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual HRESULT Render() override;

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderValue();
    HRESULT Bind_EmitterRectValue(const EMITTER_PARTICLE& Particle);

private:
    CVIBuffer_Rect* m_pVIBuffer{};

private:
    _uint m_iVIBufferLevel{};
    _wstring m_wstrVIBufferTag;

    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;

    _float2 m_fCurTexAniUV{};
    _float2 m_fCurTexAniSize{};

    _float2 m_fCurMaskAniUV{};
    _float2 m_fCurMaskAniSize{};

private:
    EffectRect::VALUES Make_RectValues();
    void Init_PropertyValue();
};

NS_END
