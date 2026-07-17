#pragma once

#include "Effect_NonParticle.h"

NS_BEGIN(Engine)

class CVIBuffer_Rect;
namespace EffectRect { struct VALUES; }

class ENGINE_DLL CEffect_Quad abstract : public CEffect_NonParticle
{
    GENERATED_BODY_ABSTRACT(CEffect_Quad)

    PROPERTY(_int, m_iRotationAxisSpace, L"Rotation Axis Space_R", L"Rot");
    PROPERTY(_bool, m_bBillboard, L"Billboard", L"Effect");
    PROPERTY(_bool, m_bTextureColorToAlpha, L"Color To Alpha_T", L"Texture Com");
    PROPERTY(_bool, m_bUseTextureUVEdgeFade, L"Use UV Edge Fade_T", L"Texture Com");
    PROPERTY(_int, m_iTextureUVEdgeFadeAxis, L"Edge Fade Axis_T", L"Texture Com");
    PROPERTY(_float, m_fTextureUVEdgeFadeStartRange, L"Edge Fade Start Range_T", L"Texture Com");
    PROPERTY(_float, m_fTextureUVEdgeFadeEndRange, L"Edge Fade End Range_T", L"Texture Com");
    PROPERTY(_float, m_fTextureUVEdgeFadePower, L"Edge Fade Power_T", L"Texture Com");

    // Sprite Animation Texture
    PROPERTY(_bool, m_bSpriteAniTexture, L"Sprite Animation Texture", L"Sprite Animation");
    
    PROPERTY(_int, m_iTexFrameX,     L"Frame X_T",   L"Sprite Animation");
    PROPERTY(_int, m_iTexFrameY,     L"Frame Y_T",   L"Sprite Animation");
        
    // Sprite Animation Mask
    PROPERTY(_bool, m_bSpriteAniMask, L"Sprite Animation Mask", L"Sprite Animation");
    
    PROPERTY(_int, m_iMaskFrameX, L"Frame X_M", L"Sprite Animation");
    PROPERTY(_int, m_iMaskFrameY, L"Frame Y_M", L"Sprite Animation");
    
public:
    struct EFFECT_QUAD_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
        // Buffer
        _uint iVIBufferLevel{};
        _wstring wstrVIBufferTag;

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

protected:
    CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Quad(const CEffect_Quad& Prototype);
    virtual ~CEffect_Quad() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual HRESULT Render() override;

protected:
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;
    virtual _bool Use_LocalRotationAxis() const override { return 1 == m_iRotationAxisSpace; }

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShaderValue();

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
