#pragma once

#include "Effect_Part.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CEffect_Mesh abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Mesh)

    // Diffuse
    PROPERTY(_bool, m_bUseDiffuseTexture,           L"Use Diffuse Textrue",     L"Diffuse");
    PROPERTY(_float2, m_vDiffuseTiling,             L"Tiling",                  L"Diffuse");
    PROPERTY(_float2, m_vDiffuseOffset,             L"Offset",                  L"Diffuse");

    PROPERTY(_bool, m_vDiffuseUVScroll,             L"UV Scroll     _",         L"Diffuse");
    PROPERTY(_float2, m_vDiffuseUVScrollCount,      L"UV Scroll Count",         L"Diffuse");

    // Unknown
    PROPERTY(_bool, m_bUseUnknownTexture,           L"Use Unknown Textrue",     L"Unknown");
    PROPERTY(_float2, m_vUnknownTiling,             L"Tiling",                  L"Unknown");
    PROPERTY(_float2, m_vUnknownOffset,             L"Offset",                  L"Unknown");

    PROPERTY(_bool, m_vUnknownUVScroll,             L"UV Scroll     _",         L"Unknown");
    PROPERTY(_float2, m_vUnknownUVScrollCount,      L"UV Scroll Count",         L"Unknown");

public:
    struct EFFECT_MESH_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
        // Model ฐüทร
        _uint iModelLevel{};
        _wstring wstrModelTag;
        _bool bUseDiffuseTexture{};
        _bool bUseUnKnownTexture{};

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

private:
    enum ShaderPass { Default, AlphaBlend, Additive, Default_Mirror, AlphaBlend_Mirror, Additive_Mirror, ShaderPass_End };

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
    _float2 m_vCurDiffuseUVOffset{};
    _float2 m_vCurUnknownUVOffset{};

protected:
    virtual void Update_UVScroll(const _float fTimeDelta, const _float fRatio) override;
    virtual void Update_EffectPart(const _float fTimeDelta, const _float fRatio) override;

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShaderValue();

private:
    CModel* m_pModelCom{};

    // Model
    _uint m_iModelLevel{};
    _wstring m_wstrModelTag;

    // Shader
    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;

private:
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END
