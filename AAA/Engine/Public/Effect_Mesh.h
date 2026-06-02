#pragma once

#include "Effect_Part.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CEffect_Mesh abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Mesh)
    PROPERTY(_bool, m_bUseDiffuseTexture,   L"Use_Diffuse_Textrue",  L"Effect_Model");
    PROPERTY(_bool, m_bUseUnKnownTexture,   L"Use_UnKnown_Textrue",  L"Effect_Model");

    PROPERTY(_bool, m_bUseTextureCom,       L"Use_TextureCom",      L"Effect_Model");

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

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShaderValue();

private:
    CModel* m_pModelCom{};

    // Model
    _uint m_iModelLevel{};
    _wstring m_wstrModelTag;

    // Textuue
    _uint m_iTextureLevel{};
    _wstring m_wstrTextureTag;

    // Shader
    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;


protected:
    virtual void Free() override;
};

NS_END
