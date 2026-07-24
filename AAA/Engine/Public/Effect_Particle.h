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
PROPERTY(_bool, m_bParticleDetachParent, L"Detach Parent_P", L"Particle Parent");
PROPERTY(_float, m_fParticleDetachParentRatio, L"Detach Parent Ratio_P", L"Particle Parent");

// Particle Spawn
PROPERTY(_bool, m_bParticleSpawnRandom, L"Spawn Random_P", L"Particle Spawn");
PROPERTY(_float, m_fParticleSpawnStartRatio, L"Spawn Start Ratio_P", L"Particle Spawn");
PROPERTY(_float, m_fParticleSpawnEndRatio, L"Spawn End Ratio_P", L"Particle Spawn");
PROPERTY(_float, m_fParticleLifeRatio, L"Life Ratio_P", L"Particle Spawn");

// Particle Shape
PROPERTY(_int, m_iParticleShapeType, L"Shape Type_P", L"Particle Shape"); // 0 Point, 1 Sphere, 2 Circle, 3 Box, 4 Fan
PROPERTY(_float, m_fParticleShapeRadius, L"Radius_P", L"Particle Shape - Sphere Circle Fan");
PROPERTY(_bool, m_bParticleShapeRandomRadius, L"Random Radius_P", L"Particle Shape - Sphere Circle Fan");

PROPERTY(_float, m_fParticleFanAngle, L"Angle_P", L"Particle Shape - Fan");
PROPERTY(_float, m_fParticleFanDirectionDegree, L"Direction Degree_P", L"Particle Shape - Fan"); // 0 degrees = +X, positive toward +Z

PROPERTY(_float3, m_vParticleBoxSize, L"Box Size_P", L"Particle Shape - Box");
PROPERTY(_int, m_iParticleBoxSpawnMode, L"Box Spawn Mode_P", L"Particle Shape - Box"); // 0 Volume, 1 Top

// Particle Velocity
PROPERTY(_int, m_iParticleVelocityMode, L"Velocity Mode_P", L"Particle Velocity"); // 0 Shape Outward, 1Direction, 2 Fountain
PROPERTY(_float, m_fParticleStartSpeed, L"Start Speed_P", L"Particle Velocity");

PROPERTY(_float, m_fParticleDirectionRandomStrength, L"Direction Random_P", L"Particle Velocity - Random");
PROPERTY(_float, m_fParticleStartSpeedRandomRatio, L"Speed Random Ratio_P", L"Particle Velocity - Random");

PROPERTY(_float3, m_vParticleVelocityDirection, L"Direction_P", L"Particle Velocity - Direction");

PROPERTY(_float, m_fParticleFountainSpread, L"Spread_P", L"Particle Velocity - Fountain");
PROPERTY(_float, m_fParticleFountainUpBias, L"Up Bias_P", L"Particle Velocity - Fountain");


// Particle Force
PROPERTY(_bool, m_bParticleUseAcceleration, L"Use Acceleration_P", L"Particle Force");
PROPERTY(_float3, m_vParticleAcceleration, L"Acceleration_P", L"Particle Force");

// Particle Flutter
PROPERTY(_bool, m_bParticleUseFlutter, L"Use Flutter_P", L"Particle Flutter");
PROPERTY(_float, m_fParticleFlutterAmplitude, L"Amplitude_P", L"Particle Flutter");
PROPERTY(_float, m_fParticleFlutterFrequency, L"Frequency_P", L"Particle Flutter");
PROPERTY(_float, m_fParticleFlutterRandomRatio, L"Random Ratio_P", L"Particle Flutter");

// Particle Rotation
PROPERTY(_bool, m_bParticleRandomRotation, L"Random Rotation_P", L"Particle Rotation");
PROPERTY(_float3, m_vParticleRandomRotationMin, L"Rotation Min_P", L"Particle Rotation");
PROPERTY(_float3, m_vParticleRandomRotationMax, L"Rotation Max_P", L"Particle Rotation");

PROPERTY(_bool, m_bParticleRotationOverLife, L"Rotation Over Life_P", L"Particle Rotation");
PROPERTY(_float3, m_vParticleAngularVelocity, L"Angular Velocity_P", L"Particle Rotation");

// Particle Alpha
PROPERTY(_float, m_fParticleAlpha, L"Alpha_P", L"Particle Alpha");

PROPERTY(_bool, m_bParticleAlphaChange, L"Alpha Change_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaStartValue, L"Alpha_Start_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlphaEndValue, L"Alpha_End_P", L"Particle Alpha");

PROPERTY(_bool, m_bActive_ParticleAlpha_Ratio_0, L"Alpha_Active Ratio 0_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Ratio_0, L"Alpha_Ratio 0_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Value_0, L"Alpha_Value 0_P", L"Particle Alpha");

PROPERTY(_bool, m_bActive_ParticleAlpha_Ratio_1, L"Alpha_Active Ratio 1_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Ratio_1, L"Alpha_Ratio 1_P", L"Particle Alpha");
PROPERTY(_float, m_fParticleAlpha_Value_1, L"Alpha_Value 1_P", L"Particle Alpha");

// Particle Size
PROPERTY(_bool, m_bParticleRandomSize, L"Random Size_P", L"Size");
PROPERTY(_float2, m_vParticleStartSizeRange, L"Size Range_P", L"Size");

PROPERTY(_float, m_fParticleStartSize, L"Start Size_P", L"Size");
PROPERTY(_bool, m_bParticleSizeChange, L"Size Change_P", L"Size");
PROPERTY(_float, m_fParticleSizeStartValue, L"Size_Start_P", L"Size");
PROPERTY(_float, m_fParticleSizeEndValue, L"Size_End_P", L"Size");

PROPERTY(_bool, m_bActive_ParticleSize_Ratio_0, L"Size_Active Ratio 0_P", L"Size");
PROPERTY(_float, m_fParticleSize_Ratio_0, L"Size_Ratio 0_P", L"Size");
PROPERTY(_float, m_fParticleSize_Value_0, L"Size_Value 0_P", L"Size");

PROPERTY(_bool, m_bActive_ParticleSize_Ratio_1, L"Size_Active Ratio 1_P", L"Size");
PROPERTY(_float, m_fParticleSize_Ratio_1, L"Size_Ratio 1_P", L"Size");
PROPERTY(_float, m_fParticleSize_Value_1, L"Size_Value 1_P", L"Size");

// Particle Color
PROPERTY(_float3, m_vParticleColor, L"Color_P", L"Particle Color");

PROPERTY(_bool, m_bParticleRandomColor, L"Random Color_P", L"Particle Color - Random");
PROPERTY(_uint, m_iParticleRandomColorCount, L"Random Color Count_P", L"Particle Color - Random");
PROPERTY(_float3, m_vParticleRandomColor_0, L"Random Color 0_P", L"Particle Color - Random");
PROPERTY(_float3, m_vParticleRandomColor_1, L"Random Color 1_P", L"Particle Color - Random");
PROPERTY(_float3, m_vParticleRandomColor_2, L"Random Color 2_P", L"Particle Color - Random");
PROPERTY(_float3, m_vParticleRandomColor_3, L"Random Color 3_P", L"Particle Color - Random");
PROPERTY(_float3, m_vParticleRandomColor_4, L"Random Color 4_P", L"Particle Color - Random");

PROPERTY(_bool, m_bParticleColorChange, L"Color Change_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColorStartValue, L"Color_Start_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColorEndValue, L"Color_End_P", L"Particle Color");

PROPERTY(_bool, m_bActive_ParticleColor_Ratio_0, L"Color_Active Ratio 0_P", L"Particle Color");
PROPERTY(_float, m_fParticleColor_Ratio_0, L"Color_Ratio 0_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColor_Value_0, L"Color_Value 0_P", L"Particle Color");

PROPERTY(_bool, m_bActive_ParticleColor_Ratio_1, L"Color_Active Ratio 1_P", L"Particle Color");
PROPERTY(_float, m_fParticleColor_Ratio_1, L"Color_Ratio 1_P", L"Particle Color");
PROPERTY(_float3, m_vParticleColor_Value_1, L"Color_Value 1_P", L"Particle Color");



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
        _float3 vRandomColor{ 1.f, 1.f, 1.f };

        _float3 vLocalPos{};
        _float3 vVelocity{};
        _float3 vSpawnLocalPos{};

        _float fFlutterPhase{};
        _float fFlutterAmplitude{};
        _float fFlutterFrequency{};
        _float3 vFlutterDir{};

        _float3 vBaseRotation{};
        _float3 vRotation{};
        _float3 vAngularVelocity{};

        _bool bParentDetached{};
        _bool bParentDetachPending{};
        _float fPreviousLocalRatio{ -1.f };
        _float4x4 DetachedParentWorldMatrix{};
    };

protected:
    enum ParticleShapeType
    {
        PARTICLE_SHAPE_POINT,
        PARTICLE_SHAPE_SPHERE,
        PARTICLE_SHAPE_CIRCLE,
        PARTICLE_SHAPE_BOX,
        PARTICLE_SHAPE_FAN,
        PARTICLE_SHAPE_END
    };

    enum ParticleBoxSpawnMode
    {
        PARTICLE_BOX_VOLUME,
        PARTICLE_BOX_TOP,
        PARTICLE_BOX_END
    };

    enum ParticleVelocityMode
    {
        PARTICLE_VELOCITY_SHAPE_OUTWARD,
        PARTICLE_VELOCITY_DIRECTION,
        PARTICLE_VELOCITY_FOUNTAIN,
        PARTICLE_VELOCITY_END
    };

protected:
    CEffect_Particle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Particle(const CEffect_Particle& Prototype);
    virtual ~CEffect_Particle() = default;

protected:
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Late_Update(_float fTimeDelta) override;

    virtual void    Effect_Start() override;

protected:
    virtual void On_Deserialized() override;
    virtual _bool Use_PropertyScaleForParticleSize() const override { return true; }
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;

    void Reset_Particles();
    void Update_Particles_ByContainerTime(_float fRatio);
    void Update_ParticleParentMatrices();

    _float3 Make_ParticleSpawnLocalPos() const;
    _vector Make_ParticleVelocityDirection(const PARTICLE& Particle) const;
    _vector Apply_ParticleVelocityRandom(_vector vBaseDir) const;
    _float Make_ParticleRandomSpeed() const;

    _vector Make_RandomSphereDirection() const;
    _vector Make_FountainDirection() const;
    _float3 Select_ParticleRandomColor() const;

    void Update_ParticleMove(PARTICLE& Particle, _float fRatio, _float fLocalRatio);
    void Update_ParticleAlpha(PARTICLE& Particle, _float fLocalRatio);
    void Update_ParticleSize(PARTICLE& Particle, _float fLocalRatio);
    void Update_ParticleColor(PARTICLE& Particle, _float fLocalRatio);

    _float4x4 Make_ParticleWorldMatrix(const PARTICLE& Particle) const;

protected:
    vector<PARTICLE> m_Particles;

    _float  m_fParticleLifeTime = 1.f;
    _float3 m_fPivot{};

private:
    void Init_PropertyValue();
};

NS_END
