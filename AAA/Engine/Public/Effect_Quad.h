#pragma once

#include "Effect_Part.h"

NS_BEGIN(Engine)

class CVIBuffer_Rect;

class ENGINE_DLL CEffect_Quad abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Quad)

    // Texture
    PROPERTY(_bool, m_bUseTextureCom,       L"Use_TextureCom",      L"Effect_Model");
    PROPERTY(_float2, m_vTextureTiling,     L"Texture_Tiling",      L"Effect_Model");
    PROPERTY(_float2, m_vTextureOffset,     L"Texture_Offset",      L"Effect_Model");

    PROPERTY(_bool, m_vTextureUVScroll,     L"Texture_UVScroll",    L"Effect_Model");
    PROPERTY(_float2, m_vTextureUVSpeed,    L"Texture_UVSpeed",     L"Effect_Model");

public:
    struct EFFECT_QUAD_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
        // Buffer
        _uint iVIBufferLevel{};
        _wstring wstrVIBufferTag;

        // Texture
        _bool bUseTextureCom{};
        _uint iTextureLevel{};
        _wstring wstrTextureTag;

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

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
    virtual void Update_EffectPart(const _float fTimeDelta, const _float fActiveTime, const _float fRatio) override;
    virtual void Update_UVScroll(const _float fTimeDelta);

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShaderValue();

private:
    CVIBuffer_Rect* m_pVIBuffer{};

private:
    _uint m_iVIBufferLevel{};
    _wstring m_wstrVIBufferTag;

    _uint m_iTextureLevel{};
    _wstring m_wstrTextureTag;

    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;

private:
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END
