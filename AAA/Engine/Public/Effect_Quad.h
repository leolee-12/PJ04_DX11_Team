#pragma once

#include "Effect_NonParticle.h"

NS_BEGIN(Engine)

class CVIBuffer_Rect;

class ENGINE_DLL CEffect_Quad abstract : public CEffect_NonParticle
{
    GENERATED_BODY_ABSTRACT(CEffect_Quad)

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

private:
    enum ShaderPass { Default, AlphaBlend, Additive, ShaderPass_End };

protected:
    CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Quad(const CEffect_Quad& Prototype);
    virtual ~CEffect_Quad() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

protected:
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;

    virtual void Update_EffectPart(const _float fTimeDelta, const _float fRatio) override;

    virtual void Update_TexSpriteAnimation(const _float fTimeDelta, const _float fRatio);
    virtual void Update_MaskSpriteAnimation(const _float fTimeDelta, const _float fRatio);

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
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END
