#pragma once

#include "Effect_Particle.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CEffect_MeshParticle abstract : public CEffect_Particle
{
    GENERATED_BODY_ABSTRACT(CEffect_MeshParticle)

// Diffuse
PROPERTY(_bool, m_bUseDiffuseTexture,           L"Use Diffuse Textrue_D",     L"Diffuse");
PROPERTY(_float2, m_vDiffuseTiling,             L"Tiling_D",                  L"Diffuse");
PROPERTY(_float2, m_vDiffuseOffset,             L"Offset_D",                  L"Diffuse");

PROPERTY(_bool, m_vDiffuseUVScroll,             L"UV Scroll     _D",          L"Diffuse");
PROPERTY(_float2, m_vDiffuseUVScrollCount,      L"UV Scroll Count_D",         L"Diffuse");

// Unknown
PROPERTY(_bool, m_bUseUnknownTexture,           L"Use Unknown Textrue_U",     L"Unknown");
PROPERTY(_float2, m_vUnknownTiling,             L"Tiling_U",                  L"Unknown");
PROPERTY(_float2, m_vUnknownOffset,             L"Offset_U",                  L"Unknown");

PROPERTY(_bool, m_vUnknownUVScroll,             L"UV Scroll     _U",          L"Unknown");
PROPERTY(_float2, m_vUnknownUVScrollCount,      L"UV Scroll Count_U",         L"Unknown");

public:
    struct EFFECT_MESHPARTICLE_DESC : public CEffect_Particle::EFFECT_PARTICLE_DESC
    {
        // Model
        _uint iModelLevel{};
        _wstring wstrModelTag;
        _bool bUseDiffuseTexture{};
        _bool bUseUnKnownTexture{};

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

protected:
    CEffect_MeshParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_MeshParticle(const CEffect_MeshParticle& Prototype);
    virtual ~CEffect_MeshParticle() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual void    Effect_Start() override;

protected:
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;
    virtual void Update_UVScroll(const _float fTimeDelta, const _float fRatio) override;

protected:
    _float2 m_vCurDiffuseUVOffset{};
    _float2 m_vCurUnknownUVOffset{};

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();
    HRESULT Bind_ShaderValue();

private:
    CModel* m_pModelCom{};

    _uint m_iModelLevel{};
    _wstring m_wstrModelTag;

    _uint m_iShaderLevel{};
    _wstring m_wstrShaderTag;

private:
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END