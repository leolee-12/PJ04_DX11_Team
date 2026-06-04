#pragma once

#include "Effect_Part.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CEffect_Mesh abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Mesh)

    // Diffuse
    PROPERTY(_bool, m_bUseDiffuseTexture,           L"Use_Diffuse_Textrue",     L"Effect_Model");
    PROPERTY(_float2, m_vDiffuseTiling,             L"Diffuse_Tiling",          L"Effect_Model");
    PROPERTY(_float2, m_vDiffuseOffset,             L"Diffuse_Offset",          L"Effect_Model");

    PROPERTY(_bool, m_vDiffuseUVScroll,             L"Diffuse_UVScroll", L"Effect_Model");
    PROPERTY(_float2, m_vDiffuseUVScrollCount,      L"Diffuse_UVScrollCount", L"Effect_Model");

    // Unkown
    PROPERTY(_bool, m_bUseUnknownTexture,           L"Use_Unknown_Textrue",     L"Effect_Model");
    PROPERTY(_float2, m_vUnknownTiling,             L"Unknown_Tiling",          L"Effect_Model");
    PROPERTY(_float2, m_vUnknownOffset,             L"UnKnown_Offset",          L"Effect_Model");

    PROPERTY(_bool, m_vUnknownUVScroll,             L"UnKnown_UVScroll",        L"Effect_Model");
    PROPERTY(_float2, m_vUnknownUVScrollCount,      L"UnKnown_UVScrollCount",         L"Effect_Model");

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
