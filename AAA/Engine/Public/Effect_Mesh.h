#pragma once

#include "Effect_Part.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CEffect_Mesh abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Mesh)

    // Diffuse
    PROPERTY(_bool, m_bUseDiffuseTexture,   L"Use_Diffuse_Textrue",     L"Effect_Model");
    PROPERTY(_float2, m_vDiffuseTiling,     L"Diffuse_Tiling",          L"Effect_Model");
    PROPERTY(_float2, m_vDiffuseOffset,     L"Diffuse_Offset",          L"Effect_Model");

    PROPERTY(_bool, m_vDiffuseUVScroll,     L"Diffuse_UVScroll", L"Effect_Model");
    PROPERTY(_float2, m_vDiffuseUVSpeed,    L"Diffuse_UVSpeed", L"Effect_Model");

    // Unkown
    PROPERTY(_bool, m_bUseUnknownTexture,   L"Use_Unknown_Textrue",     L"Effect_Model");
    PROPERTY(_float2, m_vUnknownTiling,     L"Unknown_Tiling",          L"Effect_Model");
    PROPERTY(_float2, m_vUnknownOffset,     L"UnKnown_Offset",          L"Effect_Model");

    PROPERTY(_bool, m_vUnknownUVScroll,     L"UnKnown_UVScroll",        L"Effect_Model");
    PROPERTY(_float2, m_vUnknownUVSpeed,    L"UnKnown_UVSpeed",         L"Effect_Model");


    // Texture
    PROPERTY(_bool, m_bUseTextureCom,       L"Use_TextureCom",          L"Effect_Model");
    PROPERTY(_float2, m_vTextureTiling,     L"Texture_Tiling",          L"Effect_Model");
    PROPERTY(_float2, m_vTextureOffset,     L"Texture_Offset",          L"Effect_Model");

    PROPERTY(_bool, m_vTextureUVScroll,     L"Texture_UVScroll",        L"Effect_Model");
    PROPERTY(_float2, m_vTextureUVSpeed,    L"Texture_UVSpeed",         L"Effect_Model");

public:
    struct EFFECT_MESH_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
        // Model 관련
        _uint iModelLevel{};
        _wstring wstrModelTag;
        _bool bUseDiffuseTexture{};
        _bool bUseUnKnownTexture{};

        // Texture 컴포넌트 관련
        _bool bUseTextureCom{};
        _uint iTextureLevel{};
        _wstring wstrTextureTag;

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

protected:
    CEffect_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Mesh(const CEffect_Mesh& Prototype);
    virtual ~CEffect_Mesh() = default;

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
    CModel* m_pModelCom{};

    // Model
    _uint m_iModelLevel{};
    _wstring m_wstrModelTag;

    // Texture
    _uint m_iTextureLevel{};
    _wstring m_wstrTextureTag;

    // Shader
    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;

private:
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END
