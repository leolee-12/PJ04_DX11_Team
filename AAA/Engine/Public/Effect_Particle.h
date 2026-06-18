#pragma once

#include "Effect_Part.h"

NS_BEGIN(Engine)

class CVIBuffer_Rect;

class ENGINE_DLL CEffect_Particle abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Particle)

// Sprite Animation Texture
PROPERTY(_bool, m_bSpriteAniTexture, L"Sprite Animation Texture", L"Sprite Animation");

PROPERTY(_int, m_iTexFrameX,     L"Frame X_T",   L"Sprite Animation");
PROPERTY(_int, m_iTexFrameY,     L"Frame Y_T",   L"Sprite Animation");

// Sprite Animation Mask
PROPERTY(_bool, m_bSpriteAniMask, L"Sprite Animation Mask", L"Sprite Animation");

PROPERTY(_int, m_iMaskFrameX, L"Frame X_M", L"Sprite Animation");
PROPERTY(_int, m_iMaskFrameY, L"Frame Y_M", L"Sprite Animation");

// Particle Alpha
PROPERTY(_bool,  m_bParticleFadeInOut,          L"Fade In Out_P",       L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaStartValue,    L"Start_P",             L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaPeakValue,     L"Peak_P",              L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaEndValue,      L"End_P",               L"Particle Alpha");
PROPERTY(_float, m_fParticleFadeInRatio,        L"Fade In Ratio_P",     L"Particle Alpha");
PROPERTY(_float, m_fParticleFadeOutRatio,       L"Fade Out Ratio_P",    L"Particle Alpha");

// Particle Spawn
PROPERTY(_bool, m_bParticleSpawnRandom, L"Spawn Random_P", L"Particle Spawn");
PROPERTY(_float, m_fParticleSpawnStartRatio, L"Spawn Start Ratio_P", L"Particle Spawn");
PROPERTY(_float, m_fParticleSpawnEndRatio, L"Spawn End Ratio_P", L"Particle Spawn");
PROPERTY(_float, m_fParticleLifeRatio, L"Life Ratio_P", L"Particle Spawn");

// Particle Move
PROPERTY(_int, m_iParticleMoveMode, L"Move Mode_P", L"Particle Move"); // 0 Spread, 1 Fountain
PROPERTY(_float, m_fParticleStartSpeed, L"Start Speed_P", L"Particle Move");
PROPERTY(_float, m_fParticleFountainSpread, L"Fountain Spread_P", L"Particle Move");
PROPERTY(_float, m_fParticleFountainUpBias, L"Fountain Up Bias_P", L"Particle Move");
PROPERTY(_float, m_fParticleFountainGravity, L"Gravity_P", L"Particle Move");

public:
    struct EFFECT_PARTICLE_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
        // Buffer
        _uint iVIBufferLevel{};
        _wstring wstrVIBufferTag;

        // Shader
        _uint iShaderLevel{};
        _wstring wstrShaderTag;
    };

protected:
    struct PARTICLE
    {
        _bool bAlive{};

        _float fStartRatio{};
        _float fEndRatio{ 1.f };

        _float fFadeInRatio{};
        _float fFadeOutRatio{};
        _float fAlphaStartValue{};
        _float fAlphaPeakValue{ 1.f };
        _float fAlphaEndValue{};
        _float fAlpha{ 1.f };

        _float3 vLocalPos{};
        _float3 vVelocity{};
        _float3 vScale{ 1.f, 1.f, 1.f };
    };

private:
    enum ShaderPass { Default, AlphaBlend, Additive, Default_Mirror, AlphaBlend_Mirror, Additive_Mirror, ShaderPass_End };

    enum ParticleMoveMode
    {
        PARTICLE_MOVE_SPREAD,
        PARTICLE_MOVE_FOUNTAIN,
        PARTICLE_MOVE_END
    };

protected:
    CEffect_Particle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Particle(const CEffect_Particle& Prototype);
    virtual ~CEffect_Particle() = default;

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
    static constexpr _uint PARTICLE_MAX_COUNT = 20;

    vector<PARTICLE> m_Particles;

    _uint   m_iParticleCount = PARTICLE_MAX_COUNT;
    _float  m_fParticleLifeTime = 1.f;
    _float  m_fParticleSize = 0.25f;

    _float3 m_fPivot{};

private:
    void Init_PropertyValue();

    void Reset_Particles();
    _float4x4 Make_ParticleWorldMatrix(const PARTICLE& Particle) const;

    void Update_Particles_ByContainerTime(_float fRatio);
    _float Evaluate_ParticleAlpha(const PARTICLE& Particle, _float fLocalRatio) const;

    _vector Make_SpreadDirection3D() const;
    _vector Make_FountainDirection() const;

    void Update_ParticleMove(PARTICLE& Particle, _float fRatio, _float fLocalRatio);

protected:
    virtual void Free() override;
};

NS_END