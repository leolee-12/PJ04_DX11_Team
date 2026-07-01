#pragma once

#include "Effect_Part.h"
#include "Effect_Allocator.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_Emitter abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_Emitter)

// Emitter Emission
PROPERTY(_uint, m_iEmitterMaxParticleCount, L"Max Count_E", L"Emitter - Emission");
PROPERTY(_float, m_fEmitterRateOverTime, L"Rate Over Time_E", L"Emitter - Emission");
PROPERTY(_uint, m_iEmitterBurstCount, L"Burst Count_E", L"Emitter - Emission");

PROPERTY(_float, m_fEmitterLifeTime, L"Life Time_E", L"Emitter - Lifetime");
PROPERTY(_bool, m_bEmitterRandomLifeTime, L"Random Life Time_E", L"Emitter - Lifetime");
PROPERTY(_float2, m_vEmitterLifeTimeRange, L"Life Time Range_E", L"Emitter - Lifetime");

// Emitter Shape
PROPERTY(_int, m_iEmitterShapeType, L"Shape Type_E", L"Emitter Shape"); // 0 Point, 1 Sphere, 2 Circle, 3 Box
PROPERTY(_float, m_fEmitterShapeRadius, L"Radius_E", L"Emitter Shape - Sphere Circle");
PROPERTY(_bool, m_bEmitterShapeRandomRadius, L"Random Radius_E", L"Emitter Shape - Sphere Circle");

PROPERTY(_float3, m_vEmitterBoxSize, L"Box Size_E", L"Emitter Shape - Box");
PROPERTY(_int, m_iEmitterBoxSpawnMode, L"Box Spawn Mode_E", L"Emitter Shape - Box"); // 0 Volume, 1 Top

// Emitter Velocity
PROPERTY(_int, m_iEmitterVelocityMode, L"Velocity Mode_E", L"Emitter Velocity"); // 0 Shape Outward, 1 Direction, 2 Fountain
PROPERTY(_float, m_fEmitterStartSpeed, L"Start Speed_E", L"Emitter Velocity");

PROPERTY(_float, m_fEmitterDirectionRandomStrength, L"Direction Random_E", L"Emitter Velocity - Random");
PROPERTY(_float, m_fEmitterStartSpeedRandomRatio, L"Speed Random Ratio_E", L"Emitter Velocity - Random");

PROPERTY(_float3, m_vEmitterVelocityDirection, L"Direction_E", L"Emitter Velocity - Direction");

PROPERTY(_float, m_fEmitterFountainSpread, L"Spread_E", L"Emitter Velocity - Fountain");
PROPERTY(_float, m_fEmitterFountainUpBias, L"Up Bias_E", L"Emitter Velocity - Fountain");

// Emitter Force
PROPERTY(_bool, m_bEmitterUseAcceleration, L"Use Acceleration_E", L"Emitter Force");
PROPERTY(_float3, m_vEmitterAcceleration, L"Acceleration_E", L"Emitter Force");

PROPERTY(_bool, m_bEmitterUseFlutter, L"Use Flutter_E", L"Emitter Flutter");
PROPERTY(_float, m_fEmitterFlutterAmplitude, L"Amplitude_E", L"Emitter Flutter");
PROPERTY(_float, m_fEmitterFlutterFrequency, L"Frequency_E", L"Emitter Flutter");
PROPERTY(_float, m_fEmitterFlutterRandomRatio, L"Random Ratio_E", L"Emitter Flutter");

// Emitter Alpha
PROPERTY(_float, m_fEmitterAlpha, L"Alpha_E", L"Emitter Alpha");

PROPERTY(_bool, m_bEmitterAlphaChange, L"Alpha Change_E", L"Emitter Alpha");
PROPERTY(_float, m_fEmitterAlphaStartValue, L"Alpha_Start_E", L"Emitter Alpha");
PROPERTY(_float, m_fEmitterAlphaEndValue, L"Alpha_End_E", L"Emitter Alpha");

PROPERTY(_bool, m_bActive_EmitterAlpha_Ratio_0, L"Alpha_Active Ratio 0_E", L"Emitter Alpha");
PROPERTY(_float, m_fEmitterAlpha_Ratio_0, L"Alpha_Ratio 0_E", L"Emitter Alpha");
PROPERTY(_float, m_fEmitterAlpha_Value_0, L"Alpha_Value 0_E", L"Emitter Alpha");

PROPERTY(_bool, m_bActive_EmitterAlpha_Ratio_1, L"Alpha_Active Ratio 1_E", L"Emitter Alpha");
PROPERTY(_float, m_fEmitterAlpha_Ratio_1, L"Alpha_Ratio 1_E", L"Emitter Alpha");
PROPERTY(_float, m_fEmitterAlpha_Value_1, L"Alpha_Value 1_E", L"Emitter Alpha");

// Emitter Size
PROPERTY(_float, m_fEmitterStartSize, L"Start Size_E", L"Emitter Size");
PROPERTY(_bool, m_bEmitterRandomSize, L"Random Size_E", L"Emitter Size");
PROPERTY(_float2, m_vEmitterStartSizeRange, L"Size Range_E", L"Emitter Size");

PROPERTY(_bool, m_bEmitterSizeChange, L"Size Change_E", L"Emitter Size");
PROPERTY(_float, m_fEmitterSizeStartValue, L"Size_Start_E", L"Emitter Size");
PROPERTY(_float, m_fEmitterSizeEndValue, L"Size_End_E", L"Emitter Size");

PROPERTY(_bool, m_bActive_EmitterSize_Ratio_0, L"Size_Active Ratio 0_E", L"Emitter Size");
PROPERTY(_float, m_fEmitterSize_Ratio_0, L"Size_Ratio 0_E", L"Emitter Size");
PROPERTY(_float, m_fEmitterSize_Value_0, L"Size_Value 0_E", L"Emitter Size");

PROPERTY(_bool, m_bActive_EmitterSize_Ratio_1, L"Size_Active Ratio 1_E", L"Emitter Size");
PROPERTY(_float, m_fEmitterSize_Ratio_1, L"Size_Ratio 1_E", L"Emitter Size");
PROPERTY(_float, m_fEmitterSize_Value_1, L"Size_Value 1_E", L"Emitter Size");

// Emitter Color
PROPERTY(_float3, m_vEmitterColor, L"Color_E", L"Emitter Color");

PROPERTY(_bool, m_bEmitterColorChange, L"Color Change_E", L"Emitter Color");
PROPERTY(_float3, m_vEmitterColorStartValue, L"Color_Start_E", L"Emitter Color");
PROPERTY(_float3, m_vEmitterColorEndValue, L"Color_End_E", L"Emitter Color");

PROPERTY(_bool, m_bActive_EmitterColor_Ratio_0, L"Color_Active Ratio 0_E", L"Emitter Color");
PROPERTY(_float, m_fEmitterColor_Ratio_0, L"Color_Ratio 0_E", L"Emitter Color");
PROPERTY(_float3, m_vEmitterColor_Value_0, L"Color_Value 0_E", L"Emitter Color");

PROPERTY(_bool, m_bActive_EmitterColor_Ratio_1, L"Color_Active Ratio 1_E", L"Emitter Color");
PROPERTY(_float, m_fEmitterColor_Ratio_1, L"Color_Ratio 1_E", L"Emitter Color");
PROPERTY(_float3, m_vEmitterColor_Value_1, L"Color_Value 1_E", L"Emitter Color");

// Emitter Rotation
PROPERTY(_bool, m_bEmitterRandomRotation, L"Random Rotation_E", L"Emitter Rotation");
PROPERTY(_float3, m_vEmitterRandomRotationMin, L"Rotation Min_E", L"Emitter Rotation");
PROPERTY(_float3, m_vEmitterRandomRotationMax, L"Rotation Max_E", L"Emitter Rotation");

PROPERTY(_bool, m_bEmitterRotationOverLife, L"Rotation Over Life_E", L"Emitter Rotation");
PROPERTY(_float3, m_vEmitterAngularVelocity, L"Angular Velocity_E", L"Emitter Rotation");

public:
    struct EFFECT_EMITTER_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
    };

protected:
    struct EMITTER_PARTICLE
    {
        _bool bAlive{};

        _float fAge{};
        _float fLifeTime{ 1.f };

        _float fAlpha{ 1.f };

        _float fBaseSize{ 1.f };
        _float3 vScale{ 1.f, 1.f, 1.f };

        _float3 vColor{ 1.f, 1.f, 1.f };

        _float3 vLocalPos{};
        _float3 vVelocity{};
        _float3 vSpawnLocalPos{};

        _float fFlutterPhase{};
        _float fFlutterAmplitude{};
        _float fFlutterFrequency{};
        _float3 vFlutterDir{};

        _float3 vRotation{};
        _float3 vAngularVelocity{};
    };

protected:
    enum ShaderPass { Default, AlphaBlend, Additive, ShaderPass_End };

    enum EmitterShapeType
    {
        EMITTER_SHAPE_POINT,
        EMITTER_SHAPE_SPHERE,
        EMITTER_SHAPE_CIRCLE,
        EMITTER_SHAPE_BOX,
        EMITTER_SHAPE_END
    };

    enum EmitterBoxSpawnMode
    {
        EMITTER_BOX_VOLUME,
        EMITTER_BOX_TOP,
        EMITTER_BOX_END
    };

    enum EmitterVelocityMode
    {
        EMITTER_VELOCITY_SHAPE_OUTWARD,
        EMITTER_VELOCITY_DIRECTION,
        EMITTER_VELOCITY_FOUNTAIN,
        EMITTER_VELOCITY_END
    };

protected:
    CEffect_Emitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Emitter(const CEffect_Emitter& Prototype);
    virtual ~CEffect_Emitter() = default;

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

    void Reset_Emitter();
    void Update_EmitterParticles(_float fTimeDelta);
    void Emit_ByRate(_float fTimeDelta);
    void Emit_Particles(_uint iEmitCount);
    _bool Spawn_EmitterParticle();
    EMITTER_PARTICLE* Find_DeadParticle();

    _float3 Make_EmitterSpawnLocalPos() const;
    _vector Make_EmitterVelocityDirection(const EMITTER_PARTICLE& Particle) const;
    _vector Apply_EmitterVelocityRandom(_vector vBaseDir) const;
    _float Make_EmitterRandomSpeed() const;

    _vector Make_RandomSphereDirection() const;
    _vector Make_FountainDirection() const;

    void Update_EmitterParticleMove(EMITTER_PARTICLE& Particle);
    void Update_EmitterParticleAlpha(EMITTER_PARTICLE& Particle, _float fLocalRatio);
    void Update_EmitterParticleSize(EMITTER_PARTICLE& Particle, _float fLocalRatio);
    void Update_EmitterParticleColor(EMITTER_PARTICLE& Particle, _float fLocalRatio);

    _bool Can_Emit() const;
    _bool Has_AliveParticle() const;

    _float Evaluate_EmitterFloatCurve(
        _float fLocalRatio, _float fFixedValue, _bool bChange,
        _float fStartValue, _float fEndValue,
        _bool bActiveRatio0, _float fRatio0, _float fValue0,
        _bool bActiveRatio1, _float fRatio1, _float fValue1) const;

    _float3 Evaluate_EmitterFloat3Curve(
        _float fLocalRatio, const _float3& vFixedValue, _bool bChange,
        const _float3& vStartValue, const _float3& vEndValue,
        _bool bActiveRatio0, _float fRatio0, const _float3& vValue0,
        _bool bActiveRatio1, _float fRatio1, const _float3& vValue1) const;

    _float4x4 Make_EmitterParticleWorldMatrix(const EMITTER_PARTICLE& Particle) const;

protected:
    vector<EMITTER_PARTICLE> m_EmitterParticles;

    _float m_fEmitterElapsedTime{};
    _float m_fEmitterSpawnAccumulator{};
    _float m_fEmitterPreviousRatio{};
    _bool m_bEmitterWasActive{};

    _float3 m_fPivot{};

private:
    HRESULT Ready_Components();
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END
