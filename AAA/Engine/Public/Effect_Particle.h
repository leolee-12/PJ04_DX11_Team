#pragma once

#include "Effect_Part.h"

#include "Effect_Allocator.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_Particle abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Particle)

// Particle
PROPERTY(_uint, m_iParticleCount, L"Count_P", L"Particle");

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

// Particle Alpha
PROPERTY(_float, m_fParticleAlpha, L"Alpha_P", L"Particle Alpha");

PROPERTY(_bool, m_bParticleAlphaChange, L"Alpha Change_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaStartValue, L"Start_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaEndValue, L"End_P", L"Particle Alpha");

PROPERTY(_bool, m_bActive_ParticleAlpha_Ratio_0, L"Active Ratio 0_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Ratio_0, L"Ratio 0_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Value_0, L"Value 0_P", L"Particle Alpha");

PROPERTY(_bool, m_bActive_ParticleAlpha_Ratio_1, L"Active Ratio 1_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Ratio_1, L"Ratio 1_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Value_1, L"Value 1_P", L"Particle Alpha");

// Particle Size
PROPERTY(_float, m_fParticleStartSize, L"Start Size_P", L"Particle Size");
PROPERTY(_bool, m_bParticleRandomSize, L"Random Size_P", L"Particle Size");
PROPERTY(_float2, m_vParticleStartSizeRange, L"Size Range_P", L"Particle Size");

PROPERTY(_bool, m_bParticleSizeChange, L"Size Change_P", L"Particle Size");
PROPERTY(_float, m_fParticleSizeStartValue, L"Start_P", L"Particle Size");
PROPERTY(_float, m_fParticleSizeEndValue, L"End_P", L"Particle Size");

PROPERTY(_bool, m_bActive_ParticleSize_Ratio_0, L"Active Ratio 0_P", L"Particle Size");
PROPERTY(_float, m_fParticleSize_Ratio_0, L"Ratio 0_P", L"Particle Size");
PROPERTY(_float, m_fParticleSize_Value_0, L"Value 0_P", L"Particle Size");

PROPERTY(_bool, m_bActive_ParticleSize_Ratio_1, L"Active Ratio 1_P", L"Particle Size");
PROPERTY(_float, m_fParticleSize_Ratio_1, L"Ratio 1_P", L"Particle Size");
PROPERTY(_float, m_fParticleSize_Value_1, L"Value 1_P", L"Particle Size");

// Particle Color
PROPERTY(_float3, m_vParticleColor, L"Color_P", L"Particle Color");

PROPERTY(_bool, m_bParticleColorChange, L"Color Change_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColorStartValue, L"Start_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColorEndValue, L"End_P", L"Particle Color");

PROPERTY(_bool, m_bActive_ParticleColor_Ratio_0, L"Active Ratio 0_P", L"Particle Color");
PROPERTY(_float, m_fParticleColor_Ratio_0, L"Ratio 0_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColor_Value_0, L"Value 0_P", L"Particle Color");

PROPERTY(_bool, m_bActive_ParticleColor_Ratio_1, L"Active Ratio 1_P", L"Particle Color");
PROPERTY(_float, m_fParticleColor_Ratio_1, L"Ratio 1_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColor_Value_1, L"Value 1_P", L"Particle Color");

public:
    struct EFFECT_PARTICLE_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
    };

protected:
    struct PARTICLE
    {
        _bool bAlive{};

        _float fStartRatio{};
        _float fEndRatio{ 1.f };

        _float fAlpha{ 1.f };

        _float  fBaseSize{ 1.f };
        _float3 vScale{ 1.f, 1.f, 1.f };

        _float3 vColor{ 1.f, 1.f, 1.f };

        _float3 vLocalPos{};
        _float3 vVelocity{};
    };

protected:
    enum ShaderPass { Default, AlphaBlend, Additive, ShaderPass_End };

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
    HRESULT Bind_ShaderValue();

protected:
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;

    void Reset_Particles();
    void Update_Particles_ByContainerTime(_float fRatio);
    _vector Make_SpreadDirection3D() const;
    _vector Make_FountainDirection() const;

    void Update_ParticleMove(PARTICLE& Particle, _float fRatio, _float fLocalRatio);
    void Update_ParticleAlpha(PARTICLE& Particle, _float fLocalRatio);
    void Update_ParticleSize(PARTICLE& Particle, _float fLocalRatio);
    void Update_ParticleColor(PARTICLE& Particle, _float fLocalRatio);

    _float Evaluate_ParticleFloatCurve(
        _float fLocalRatio, _float fFixedValue, _bool bChange,
        _float fStartValue, _float fEndValue,
        _bool bActiveRatio0, _float fRatio0, _float fValue0,
        _bool bActiveRatio1, _float fRatio1, _float fValue1) const;

    _float3 Evaluate_ParticleFloat3Curve(
        _float fLocalRatio, const _float3& vFixedValue, _bool bChange,
        const _float3& vStartValue, const _float3& vEndValue,
        _bool bActiveRatio0, _float fRatio0, const _float3& vValue0,
        _bool bActiveRatio1, _float fRatio1, const _float3& vValue1) const;

    _float4x4 Make_ParticleWorldMatrix(const PARTICLE& Particle) const;

protected:
    vector<PARTICLE> m_Particles;

    _float  m_fParticleLifeTime = 1.f;

    _float3 m_fPivot{};

private:
    HRESULT Ready_Components();
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END