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

// Circle UV - Texture Com
PROPERTY(_bool,  m_bTextureCircleUVAnim,         L"Circle UV     _T",          L"Texture Com");
PROPERTY(_float, m_fTextureCircleUVStartRatio,   L"Circle Start Ratio_T", L"Texture Com");
PROPERTY(_float, m_fTextureCircleUVEndRatio,     L"Circle End Ratio_T",   L"Texture Com");
PROPERTY(_float, m_fTextureCircleUVStartDegree,  L"Circle Start Degree_T",     L"Texture Com");
PROPERTY(_bool,  m_bTextureCircleUVClockwise,    L"Circle Clockwise_T",        L"Texture Com");

// Circle UV - Mask Com
PROPERTY(_bool,  m_bMaskCircleUVAnim,            L"Circle UV     _M",          L"Mask Com");
PROPERTY(_float, m_fMaskCircleUVStartRatio,      L"Circle Start Ratio_M", L"Mask Com");
PROPERTY(_float, m_fMaskCircleUVEndRatio,        L"Circle End Ratio_M",   L"Mask Com");
PROPERTY(_float, m_fMaskCircleUVStartDegree,     L"Circle Start Degree_M",     L"Mask Com");
PROPERTY(_bool,  m_bMaskCircleUVClockwise,       L"Circle Clockwise_M",        L"Mask Com");

// Circle UV - Diffuse
PROPERTY(_bool,  m_bDiffuseCircleUVAnim,         L"Circle UV     _D",          L"Diffuse");
PROPERTY(_float, m_fDiffuseCircleUVStartRatio,   L"Circle Start Ratio_D", L"Diffuse");
PROPERTY(_float, m_fDiffuseCircleUVEndRatio,     L"Circle End Ratio_D",   L"Diffuse");
PROPERTY(_float, m_fDiffuseCircleUVStartDegree,  L"Circle Start Degree_D",     L"Diffuse");
PROPERTY(_bool,  m_bDiffuseCircleUVClockwise,    L"Circle Clockwise_D",        L"Diffuse");

// Circle UV - Unknown
PROPERTY(_bool,  m_bUnknownCircleUVAnim,         L"Circle UV     _U",          L"Unknown");
PROPERTY(_float, m_fUnknownCircleUVStartRatio,   L"Circle Start Ratio_U", L"Unknown");
PROPERTY(_float, m_fUnknownCircleUVEndRatio,     L"Circle End Ratio_U",   L"Unknown");
PROPERTY(_float, m_fUnknownCircleUVStartDegree,  L"Circle Start Degree_U",     L"Unknown");
PROPERTY(_bool,  m_bUnknownCircleUVClockwise,    L"Circle Clockwise_U",        L"Unknown");

// Linear UV - Texture Com
PROPERTY(_bool,  m_bTextureLinearUVAnim,         L"Linear UV     _T",          L"Texture Com");
PROPERTY(_int,   m_iTextureLinearUVAxis,         L"Linear Axis_T",             L"Texture Com");
PROPERTY(_bool,  m_bTextureLinearUVReverse,      L"Linear Reverse_T",          L"Texture Com");
PROPERTY(_float, m_fTextureLinearUVStartRatio,   L"Linear Start Ratio_T",      L"Texture Com");
PROPERTY(_float, m_fTextureLinearUVEndRatio,     L"Linear End Ratio_T",        L"Texture Com");

// Linear UV - Mask Com
PROPERTY(_bool,  m_bMaskLinearUVAnim,            L"Linear UV     _M",          L"Mask Com");
PROPERTY(_int,   m_iMaskLinearUVAxis,            L"Linear Axis_M",             L"Mask Com");
PROPERTY(_bool,  m_bMaskLinearUVReverse,         L"Linear Reverse_M",          L"Mask Com");
PROPERTY(_float, m_fMaskLinearUVStartRatio,      L"Linear Start Ratio_M",      L"Mask Com");
PROPERTY(_float, m_fMaskLinearUVEndRatio,        L"Linear End Ratio_M",        L"Mask Com");

// Linear UV - Diffuse
PROPERTY(_bool,  m_bDiffuseLinearUVAnim,         L"Linear UV     _D",          L"Diffuse");
PROPERTY(_int,   m_iDiffuseLinearUVAxis,         L"Linear Axis_D",             L"Diffuse");
PROPERTY(_bool,  m_bDiffuseLinearUVReverse,      L"Linear Reverse_D",          L"Diffuse");
PROPERTY(_float, m_fDiffuseLinearUVStartRatio,   L"Linear Start Ratio_D",      L"Diffuse");
PROPERTY(_float, m_fDiffuseLinearUVEndRatio,     L"Linear End Ratio_D",        L"Diffuse");

// Linear UV - Unknown
PROPERTY(_bool,  m_bUnknownLinearUVAnim,         L"Linear UV     _U",          L"Unknown");
PROPERTY(_int,   m_iUnknownLinearUVAxis,         L"Linear Axis_U",             L"Unknown");
PROPERTY(_bool,  m_bUnknownLinearUVReverse,      L"Linear Reverse_U",          L"Unknown");
PROPERTY(_float, m_fUnknownLinearUVStartRatio,   L"Linear Start Ratio_U",      L"Unknown");
PROPERTY(_float, m_fUnknownLinearUVEndRatio,     L"Linear End Ratio_U",        L"Unknown");

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

    _float m_fTextureCircleUVRatio{ 1.f };
    _float m_fMaskCircleUVRatio{ 1.f };
    _float m_fDiffuseCircleUVRatio{ 1.f };
    _float m_fUnknownCircleUVRatio{ 1.f };

    _float m_fTextureLinearUVRatio{ 0.f };
    _float m_fMaskLinearUVRatio{ 0.f };
    _float m_fDiffuseLinearUVRatio{ 0.f };
    _float m_fUnknownLinearUVRatio{ 0.f };

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