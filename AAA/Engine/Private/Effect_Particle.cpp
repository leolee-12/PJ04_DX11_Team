#include "Effect_Particle.h"

#include "GameInstance.h"

CEffect_Particle::CEffect_Particle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Particle::CEffect_Particle(const CEffect_Particle& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Particle::Initialize(void* pArg)
{
    EFFECT_PARTICLE_DESC* pDesc = static_cast<EFFECT_PARTICLE_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    Reset_Particles();

    return S_OK;
}

void CEffect_Particle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    Compute_CombinedWorldMatrix();
    Update_ParticleParentMatrices();
}

void CEffect_Particle::Effect_Start()
{
    __super::Effect_Start();

    Reset_Particles();
}

void CEffect_Particle::On_Deserialized()
{
    __super::On_Deserialized();
    Reset_Particles();
}

void CEffect_Particle::Init_PropertyValue()
{
    // Particle
    m_iParticleCount = 20;
    m_bParticleDetachParent = false;
    m_fParticleDetachParentRatio = 1.f;

    // Particle Spawn
    m_bParticleSpawnRandom = true;
    m_fParticleSpawnStartRatio = 0.f;
    m_fParticleSpawnEndRatio = 0.7f;
    m_fParticleLifeRatio = 0.25f;

    // Particle Shape
    m_iParticleShapeType = PARTICLE_SHAPE_POINT;

    m_fParticleShapeRadius = 1.f;
    m_bParticleShapeRandomRadius = false;

    m_fParticleFanAngle = 90.f;
    m_fParticleFanDirectionDegree = 0.f;

    m_vParticleBoxSize = { 5.f, 3.f, 5.f };
    m_iParticleBoxSpawnMode = PARTICLE_BOX_TOP;

    // Particle Velocity
    m_iParticleVelocityMode = PARTICLE_VELOCITY_SHAPE_OUTWARD;
    m_fParticleStartSpeed = 6.f;

    m_vParticleVelocityDirection = { 0.f, -1.f, 0.f };

    m_fParticleFountainSpread = 1.f;
    m_fParticleFountainUpBias = 1.5f;

    m_fParticleDirectionRandomStrength = 0.f;
    m_fParticleStartSpeedRandomRatio = 0.f;

    // Particle Force
    m_bParticleUseAcceleration = false;
    m_vParticleAcceleration = { 0.f, 0.f, 0.f };

    // Particle Flutter
    m_bParticleUseFlutter = false;
    m_fParticleFlutterAmplitude = 0.3f;
    m_fParticleFlutterFrequency = 1.5f;
    m_fParticleFlutterRandomRatio = 0.3f;

    // Particle Rotation
    m_bParticleRandomRotation = false;
    m_vParticleRandomRotationMin = { 0.f, 0.f, 0.f };
    m_vParticleRandomRotationMax = { 0.f, 360.f, 0.f };

    m_bParticleRotationOverLife = false;
    m_vParticleAngularVelocity = { 0.f, 0.f, 0.f };

    // Particle Alpha
    m_fParticleAlpha = 1.f;

    m_bParticleAlphaChange = true;
    m_fParticleAlphaStartValue = 0.f;
    m_fParticleAlphaEndValue = 0.f;

    m_bActive_ParticleAlpha_Ratio_0 = true;
    m_fParticleAlpha_Ratio_0 = 0.3f;
    m_fParticleAlpha_Value_0 = 1.f;

    m_bActive_ParticleAlpha_Ratio_1 = true;
    m_fParticleAlpha_Ratio_1 = 0.7f;
    m_fParticleAlpha_Value_1 = 1.f;

    // Particle Size
    m_fParticleStartSize = 0.5f;

    m_bParticleRandomSize = false;
    m_vParticleStartSizeRange = { 0.3f, 0.7f };

    m_bParticleSizeChange = true;
    m_fParticleSizeStartValue = 0.1f;
    m_fParticleSizeEndValue = 0.1f;

    m_bActive_ParticleSize_Ratio_0 = true;
    m_fParticleSize_Ratio_0 = 0.5f;
    m_fParticleSize_Value_0 = 1.f;

    m_bActive_ParticleSize_Ratio_1 = false;
    m_fParticleSize_Ratio_1 = 0.75f;
    m_fParticleSize_Value_1 = 1.f;

    // Particle Color
    m_vParticleColor = { 1.f, 1.f, 1.f };

    m_bParticleRandomColor = false;
    m_iParticleRandomColorCount = 5;
    m_vParticleRandomColor_0 = { 1.f, 1.f, 1.f };
    m_vParticleRandomColor_1 = { 1.f, 1.f, 1.f };
    m_vParticleRandomColor_2 = { 1.f, 1.f, 1.f };
    m_vParticleRandomColor_3 = { 1.f, 1.f, 1.f };
    m_vParticleRandomColor_4 = { 1.f, 1.f, 1.f };

    m_bParticleColorChange = false;
    m_vParticleColorStartValue = { 1.f, 1.f, 1.f };
    m_vParticleColorEndValue = { 1.f, 1.f, 1.f };

    m_bActive_ParticleColor_Ratio_0 = false;
    m_fParticleColor_Ratio_0 = 0.5f;
    m_vParticleColor_Value_0 = { 1.f, 1.f, 1.f };

    m_bActive_ParticleColor_Ratio_1 = false;
    m_fParticleColor_Ratio_1 = 0.75f;
    m_vParticleColor_Value_1 = { 1.f, 1.f, 1.f };
}

void CEffect_Particle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    if (m_bOrbitChange == true)
        Set_LocalPositionFromProperty();
    Update_Orbit(fRatio);

    Update_Particles_ByContainerTime(fRatio);
}

void CEffect_Particle::Reset_Particles()
{
    if (m_iParticleCount < 1)
        m_iParticleCount = 1;

    m_Particles.clear();
    m_Particles.resize(m_iParticleCount);

    for (_uint i = 0; i < m_iParticleCount; ++i)
    {
        PARTICLE& Particle = m_Particles[i];

        Particle.bParentDetached = false;
        Particle.bParentDetachPending = false;
        Particle.fPreviousLocalRatio = -1.f;
        XMStoreFloat4x4(&Particle.DetachedParentWorldMatrix, XMMatrixIdentity());

        _float fLifeRatio = m_fParticleLifeRatio;
        Helper::FloatClamp(fLifeRatio, Helper::fEpsilon, 1.f);

        _float fSpawnStartRatio = m_fParticleSpawnStartRatio;
        _float fSpawnEndRatio = m_fParticleSpawnEndRatio;

        const _float fMaxSpawnRatio = 1.f - fLifeRatio;

        Helper::FloatClamp(fSpawnStartRatio, 0.f, fMaxSpawnRatio);
        Helper::FloatClamp(fSpawnEndRatio, 0.f, fMaxSpawnRatio);

        if (fSpawnEndRatio < fSpawnStartRatio)
            std::swap(fSpawnStartRatio, fSpawnEndRatio);

        Particle.fStartRatio = m_bParticleSpawnRandom
            ? m_pGameInstance_Proxy->RandomFloat(fSpawnStartRatio, fSpawnEndRatio)
            : fSpawnStartRatio;

        Particle.fEndRatio = Particle.fStartRatio + fLifeRatio;

        Helper::IntClamp(m_iParticleShapeType, PARTICLE_SHAPE_POINT, PARTICLE_SHAPE_END - 1);
        Helper::IntClamp(m_iParticleVelocityMode, PARTICLE_VELOCITY_SHAPE_OUTWARD, PARTICLE_VELOCITY_END - 1);

        Particle.vSpawnLocalPos = Make_ParticleSpawnLocalPos();
        Particle.vLocalPos = Particle.vSpawnLocalPos;

        _vector vDir = Make_ParticleVelocityDirection(Particle);
        vDir = Apply_ParticleVelocityRandom(vDir);

        const _float fSpeed = Make_ParticleRandomSpeed();

        XMStoreFloat3(&Particle.vVelocity, vDir * fSpeed);

        _float fParticleSize = m_fParticleStartSize;

        if (m_bParticleRandomSize == true)
        {
            _float fMinSize = m_vParticleStartSizeRange.x;
            _float fMaxSize = m_vParticleStartSizeRange.y;

            if (fMaxSize < fMinSize)
                std::swap(fMinSize, fMaxSize);

            if (fMinSize < Helper::fEpsilon)
                fMinSize = Helper::fEpsilon;

            if (fMaxSize < Helper::fEpsilon)
                fMaxSize = Helper::fEpsilon;

            fParticleSize = m_pGameInstance_Proxy->RandomFloat(fMinSize, fMaxSize);
        }

        if (fParticleSize < Helper::fEpsilon)
            fParticleSize = Helper::fEpsilon;

        Particle.fBaseSize = fParticleSize;
        Particle.vScale = { fParticleSize, fParticleSize, fParticleSize };

        Particle.fAlpha = m_fParticleAlpha;
        Particle.vColor = m_vParticleColor;
        Particle.vRandomColor = m_bParticleRandomColor == true
            ? Select_ParticleRandomColor()
            : _float3{ 1.f, 1.f, 1.f };

        Particle.fFlutterPhase = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

        _float fFlutterRandomRatio = m_fParticleFlutterRandomRatio;
        Helper::FloatClamp(fFlutterRandomRatio, 0.f, 1.f);

        _float fFlutterAmpMin = m_fParticleFlutterAmplitude * (1.f - fFlutterRandomRatio);
        _float fFlutterAmpMax = m_fParticleFlutterAmplitude * (1.f + fFlutterRandomRatio);

        if (fFlutterAmpMax < fFlutterAmpMin)
            std::swap(fFlutterAmpMin, fFlutterAmpMax);

        Particle.fFlutterAmplitude = m_pGameInstance_Proxy->RandomFloat(fFlutterAmpMin, fFlutterAmpMax);

        _float fFlutterFreqMin = m_fParticleFlutterFrequency * (1.f - fFlutterRandomRatio);
        _float fFlutterFreqMax = m_fParticleFlutterFrequency * (1.f + fFlutterRandomRatio);

        if (fFlutterFreqMax < fFlutterFreqMin)
            std::swap(fFlutterFreqMin, fFlutterFreqMax);

        Particle.fFlutterFrequency = m_pGameInstance_Proxy->RandomFloat(fFlutterFreqMin, fFlutterFreqMax);

        const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
        Particle.vFlutterDir = { cosf(fTheta), 0.f, sinf(fTheta) };

        Particle.vBaseRotation = { 0.f, 0.f, 0.f };

        if (m_bParticleRandomRotation == true)
        {
            _float3 vMin = m_vParticleRandomRotationMin;
            _float3 vMax = m_vParticleRandomRotationMax;

            if (vMax.x < vMin.x) std::swap(vMin.x, vMax.x);
            if (vMax.y < vMin.y) std::swap(vMin.y, vMax.y);
            if (vMax.z < vMin.z) std::swap(vMin.z, vMax.z);

            Particle.vBaseRotation.x = m_pGameInstance_Proxy->RandomFloat(vMin.x, vMax.x);
            Particle.vBaseRotation.y = m_pGameInstance_Proxy->RandomFloat(vMin.y, vMax.y);
            Particle.vBaseRotation.z = m_pGameInstance_Proxy->RandomFloat(vMin.z, vMax.z);
        }

        Particle.vRotation = Particle.vBaseRotation;
        Particle.vAngularVelocity = m_vParticleAngularVelocity;
    }
}

void CEffect_Particle::Update_Particles_ByContainerTime(_float fRatio)
{
    for (PARTICLE& Particle : m_Particles)
    {
        _float fLocalRatio = (fRatio - Particle.fStartRatio) / (Particle.fEndRatio - Particle.fStartRatio);

        if (fLocalRatio < 0.f || fLocalRatio > 1.f)
        {
            Particle.bAlive = false;
            Particle.bParentDetached = false;
            Particle.bParentDetachPending = false;
            Particle.fPreviousLocalRatio = -1.f;
            continue;
        }

        if (Particle.bAlive == false ||
            fLocalRatio + Helper::fEpsilon < Particle.fPreviousLocalRatio)
        {
            Particle.bParentDetached = false;
            Particle.bParentDetachPending = false;
        }

        Particle.bAlive = true;
        Particle.fPreviousLocalRatio = fLocalRatio;

        if (m_bParticleDetachParent == true &&
            Particle.bParentDetached == false &&
            Particle.bParentDetachPending == false)
        {
            _float fDetachRatio = m_fParticleDetachParentRatio;
            Helper::FloatClamp(fDetachRatio, 0.f, 1.f);

            if (fLocalRatio >= fDetachRatio)
                Particle.bParentDetachPending = true;
        }

        Update_ParticleMove(Particle, fRatio, fLocalRatio);
        Update_ParticleAlpha(Particle, fLocalRatio);
        Update_ParticleSize(Particle, fLocalRatio);
        Update_ParticleColor(Particle, fLocalRatio);
    }
}

void CEffect_Particle::Update_ParticleParentMatrices()
{
    for (PARTICLE& Particle : m_Particles)
    {
        if (Particle.bAlive == false || Particle.bParentDetachPending == false)
            continue;

        Particle.DetachedParentWorldMatrix = m_CombinedWorldMatrix;
        Particle.bParentDetached = true;
        Particle.bParentDetachPending = false;
    }
}

_float3 CEffect_Particle::Make_ParticleSpawnLocalPos() const
{
    _int iShapeType = m_iParticleShapeType;
    Helper::IntClamp(iShapeType, PARTICLE_SHAPE_POINT, PARTICLE_SHAPE_END - 1);

    switch (iShapeType)
    {
    case PARTICLE_SHAPE_SPHERE:
    {
        _float fRadius = m_fParticleShapeRadius;

        if (fRadius < 0.f)
            fRadius = 0.f;

        if (m_bParticleShapeRandomRadius == true)
            fRadius = m_pGameInstance_Proxy->RandomFloat(0.f, fRadius);

        _vector vDir = Make_RandomSphereDirection();

        _float3 vOffset{};
        XMStoreFloat3(&vOffset, vDir * fRadius);

        return {
            m_fPivot.x + vOffset.x,
            m_fPivot.y + vOffset.y,
            m_fPivot.z + vOffset.z
        };
    }

    case PARTICLE_SHAPE_CIRCLE:
    {
        _float fRadius = m_fParticleShapeRadius;

        if (fRadius < 0.f)
            fRadius = 0.f;

        if (m_bParticleShapeRandomRadius == true)
            fRadius = m_pGameInstance_Proxy->RandomFloat(0.f, fRadius);

        const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

        return {
            m_fPivot.x + cosf(fTheta) * fRadius,
            m_fPivot.y,
            m_fPivot.z + sinf(fTheta) * fRadius
        };
    }

    case PARTICLE_SHAPE_FAN:
    {
        _float fRadius = m_fParticleShapeRadius;

        if (fRadius < 0.f)
            fRadius = 0.f;

        if (m_bParticleShapeRandomRadius == true)
            fRadius = m_pGameInstance_Proxy->RandomFloat(0.f, fRadius);

        _float fFanAngle = fabsf(m_fParticleFanAngle);
        Helper::FloatClamp(fFanAngle, 0.f, 360.f);

        const _float fCenterRadian = XMConvertToRadians(m_fParticleFanDirectionDegree);
        const _float fHalfRadian = XMConvertToRadians(fFanAngle * 0.5f);
        const _float fTheta = m_pGameInstance_Proxy->RandomFloat(
            fCenterRadian - fHalfRadian,
            fCenterRadian + fHalfRadian);

        return {
            m_fPivot.x + cosf(fTheta) * fRadius,
            m_fPivot.y,
            m_fPivot.z + sinf(fTheta) * fRadius
        };
    }

    case PARTICLE_SHAPE_BOX:
    {
        const _float fHalfX = fabsf(m_vParticleBoxSize.x) * 0.5f;
        const _float fHalfY = fabsf(m_vParticleBoxSize.y) * 0.5f;
        const _float fHalfZ = fabsf(m_vParticleBoxSize.z) * 0.5f;

        _int iBoxSpawnMode = m_iParticleBoxSpawnMode;
        Helper::IntClamp(iBoxSpawnMode, PARTICLE_BOX_VOLUME, PARTICLE_BOX_END - 1);

        _float3 vSpawnPos{};
        vSpawnPos.x = m_fPivot.x + m_pGameInstance_Proxy->RandomFloat(-fHalfX, fHalfX);
        vSpawnPos.z = m_fPivot.z + m_pGameInstance_Proxy->RandomFloat(-fHalfZ, fHalfZ);

        if (iBoxSpawnMode == PARTICLE_BOX_TOP)
            vSpawnPos.y = m_fPivot.y + fHalfY;
        else
            vSpawnPos.y = m_fPivot.y + m_pGameInstance_Proxy->RandomFloat(-fHalfY, fHalfY);

        return vSpawnPos;
    }

    case PARTICLE_SHAPE_POINT:
    default:
        return m_fPivot;
    }
}

_vector CEffect_Particle::Make_ParticleVelocityDirection(const PARTICLE& Particle) const
{
    _int iVelocityMode = m_iParticleVelocityMode;
    Helper::IntClamp(iVelocityMode, PARTICLE_VELOCITY_SHAPE_OUTWARD, PARTICLE_VELOCITY_END - 1);

    switch (iVelocityMode)
    {
    case PARTICLE_VELOCITY_DIRECTION:
    {
        _vector vDir = XMLoadFloat3(&m_vParticleVelocityDirection);

        if (XMVectorGetX(XMVector3LengthSq(vDir)) <= Helper::fEpsilon)
            vDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);

        return XMVector3Normalize(vDir);
    }

    case PARTICLE_VELOCITY_FOUNTAIN:
        return Make_FountainDirection();

    case PARTICLE_VELOCITY_SHAPE_OUTWARD:
    default:
    {
        _vector vPivot = XMLoadFloat3(&m_fPivot);
        _vector vSpawn = XMLoadFloat3(&Particle.vSpawnLocalPos);
        _vector vDir = vSpawn - vPivot;

        if (XMVectorGetX(XMVector3LengthSq(vDir)) <= Helper::fEpsilon)
            return Make_RandomSphereDirection();

        return XMVector3Normalize(vDir);
    }
    }
}

_vector CEffect_Particle::Apply_ParticleVelocityRandom(_vector vBaseDir) const
{
    _float fRandomStrength = m_fParticleDirectionRandomStrength;
    Helper::FloatClamp(fRandomStrength, 0.f, 1.f);

    if (fRandomStrength <= Helper::fEpsilon)
        return XMVector3Normalize(vBaseDir);

    _vector vRandomDir = Make_RandomSphereDirection();

    _vector vResult = XMVector3Normalize(
        vBaseDir * (1.f - fRandomStrength) +
        vRandomDir * fRandomStrength);

    if (XMVectorGetX(XMVector3LengthSq(vResult)) <= Helper::fEpsilon)
        return XMVector3Normalize(vBaseDir);

    return vResult;
}

_float CEffect_Particle::Make_ParticleRandomSpeed() const
{
    _float fRandomRatio = m_fParticleStartSpeedRandomRatio;
    Helper::FloatClamp(fRandomRatio, 0.f, 1.f);

    if (fRandomRatio <= Helper::fEpsilon)
        return m_fParticleStartSpeed;

    _float fMinSpeed = m_fParticleStartSpeed * (1.f - fRandomRatio);
    _float fMaxSpeed = m_fParticleStartSpeed * (1.f + fRandomRatio);

    if (fMaxSpeed < fMinSpeed)
        std::swap(fMinSpeed, fMaxSpeed);

    return m_pGameInstance_Proxy->RandomFloat(fMinSpeed, fMaxSpeed);
}

_vector CEffect_Particle::Make_RandomSphereDirection() const
{
    const _float fY = m_pGameInstance_Proxy->RandomFloat(-1.f, 1.f);
    const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

    const _float fRadius = sqrtf(max(0.f, 1.f - fY * fY));
    const _float fX = cosf(fTheta) * fRadius;
    const _float fZ = sinf(fTheta) * fRadius;

    return XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));
}

_vector CEffect_Particle::Make_FountainDirection() const
{
    _float fSpread = m_fParticleFountainSpread;
    _float fUpBias = m_fParticleFountainUpBias;

    Helper::FloatClamp(fSpread, 0.f, 10.f);

    if (fUpBias < Helper::fEpsilon)
        fUpBias = Helper::fEpsilon;

    const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
    const _float fHorizontal = m_pGameInstance_Proxy->RandomFloat(0.f, fSpread);

    const _float fX = cosf(fTheta) * fHorizontal;
    const _float fZ = sinf(fTheta) * fHorizontal;

    return XMVector3Normalize(XMVectorSet(fX, fUpBias, fZ, 0.f));
}

_float3 CEffect_Particle::Select_ParticleRandomColor() const
{
    _uint iColorCount = m_iParticleRandomColorCount;
    if (iColorCount < 1)
        iColorCount = 1;
    if (iColorCount > 5)
        iColorCount = 5;

    _uint iColorIndex = static_cast<_uint>(
        m_pGameInstance_Proxy->RandomFloat(0.f, static_cast<_float>(iColorCount)));

    if (iColorIndex >= iColorCount)
        iColorIndex = iColorCount - 1;

    switch (iColorIndex)
    {
    case 0: return m_vParticleRandomColor_0;
    case 1: return m_vParticleRandomColor_1;
    case 2: return m_vParticleRandomColor_2;
    case 3: return m_vParticleRandomColor_3;
    case 4: return m_vParticleRandomColor_4;
    default: return m_vParticleRandomColor_0;
    }
}

void CEffect_Particle::Update_ParticleMove(PARTICLE& Particle, _float fRatio, _float fLocalRatio)
{
    Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

    const _float fElapsedRatio = fRatio - Particle.fStartRatio;
    const _float fElapsedTime = fElapsedRatio * Get_PartDuration();

    _float3 vAcceleration{};

    if (m_bParticleUseAcceleration == true)
        vAcceleration = m_vParticleAcceleration;

    Particle.vLocalPos.x =
        Particle.vSpawnLocalPos.x +
        Particle.vVelocity.x * fElapsedTime +
        0.5f * vAcceleration.x * fElapsedTime * fElapsedTime;

    Particle.vLocalPos.y =
        Particle.vSpawnLocalPos.y +
        Particle.vVelocity.y * fElapsedTime +
        0.5f * vAcceleration.y * fElapsedTime * fElapsedTime;

    Particle.vLocalPos.z =
        Particle.vSpawnLocalPos.z +
        Particle.vVelocity.z * fElapsedTime +
        0.5f * vAcceleration.z * fElapsedTime * fElapsedTime;

    if (m_bParticleUseFlutter == true)
    {
        const _float fFlutter =
            (sinf(fElapsedTime * XM_2PI * Particle.fFlutterFrequency + Particle.fFlutterPhase) -
                sinf(Particle.fFlutterPhase)) *
            Particle.fFlutterAmplitude;

        Particle.vLocalPos.x += Particle.vFlutterDir.x * fFlutter;
        Particle.vLocalPos.z += Particle.vFlutterDir.z * fFlutter;
    }

    if (m_bParticleRotationOverLife == true)
    {
        Particle.vRotation.x = Particle.vBaseRotation.x + Particle.vAngularVelocity.x * fElapsedTime;
        Particle.vRotation.y = Particle.vBaseRotation.y + Particle.vAngularVelocity.y * fElapsedTime;
        Particle.vRotation.z = Particle.vBaseRotation.z + Particle.vAngularVelocity.z * fElapsedTime;
    }
    else
    {
        Particle.vRotation = Particle.vBaseRotation;
    }
}

void CEffect_Particle::Update_ParticleAlpha(PARTICLE& Particle, _float fLocalRatio)
{
    _float fAlpha = Evaluate_FloatCurve(
        fLocalRatio, m_fParticleAlpha, m_bParticleAlphaChange,
        m_fParticleAlphaStartValue, m_fParticleAlphaEndValue,
        m_bActive_ParticleAlpha_Ratio_0, m_fParticleAlpha_Ratio_0, m_fParticleAlpha_Value_0,
        m_bActive_ParticleAlpha_Ratio_1, m_fParticleAlpha_Ratio_1, m_fParticleAlpha_Value_1,
        true);

    fAlpha *= Get_FadeOutAlpha();
    Helper::FloatClamp(fAlpha, 0.f, 1.f);

    Particle.fAlpha = fAlpha;
}

void CEffect_Particle::Update_ParticleSize(PARTICLE& Particle, _float fLocalRatio)
{
    _float fSizeRatio = Evaluate_FloatCurve(
        fLocalRatio, 1.f, m_bParticleSizeChange,
        m_fParticleSizeStartValue, m_fParticleSizeEndValue,
        m_bActive_ParticleSize_Ratio_0, m_fParticleSize_Ratio_0, m_fParticleSize_Value_0,
        m_bActive_ParticleSize_Ratio_1, m_fParticleSize_Ratio_1, m_fParticleSize_Value_1,
        true);

    _float fSize = Particle.fBaseSize * fSizeRatio;

    if (fSize < Helper::fEpsilon)
        fSize = Helper::fEpsilon;

    Particle.vScale = { fSize, fSize, fSize };
}

void CEffect_Particle::Update_ParticleColor(PARTICLE& Particle, _float fLocalRatio)
{
    if (m_bParticleRandomColor == true && m_bParticleColorChange == false)
    {
        Particle.vColor = Particle.vRandomColor;
        return;
    }

    _float3 vColor = Evaluate_Float3Curve(
        fLocalRatio, m_vParticleColor, m_bParticleColorChange,
        m_vParticleColorStartValue, m_vParticleColorEndValue,
        m_bActive_ParticleColor_Ratio_0, m_fParticleColor_Ratio_0, m_vParticleColor_Value_0,
        m_bActive_ParticleColor_Ratio_1, m_fParticleColor_Ratio_1, m_vParticleColor_Value_1,
        true);

    if (m_bParticleRandomColor == true)
    {
        vColor.x *= Particle.vRandomColor.x;
        vColor.y *= Particle.vRandomColor.y;
        vColor.z *= Particle.vRandomColor.z;
    }

    Particle.vColor = vColor;
}

_float4x4 CEffect_Particle::Make_ParticleWorldMatrix(const PARTICLE& Particle) const
{
    _matrix matScale = XMMatrixScaling(
        Particle.vScale.x,
        Particle.vScale.y,
        Particle.vScale.z);

    _matrix matRotation = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(Particle.vRotation.x),
        XMConvertToRadians(Particle.vRotation.y),
        XMConvertToRadians(Particle.vRotation.z));

    _matrix matTranslation = XMMatrixTranslation(
        Particle.vLocalPos.x,
        Particle.vLocalPos.y,
        Particle.vLocalPos.z);

    const _float4x4& ParentWorldMatrix =
        Particle.bParentDetached == true
        ? Particle.DetachedParentWorldMatrix
        : m_CombinedWorldMatrix;

    _matrix matWorld =
        matScale *
        matRotation *
        matTranslation *
        XMLoadFloat4x4(&ParentWorldMatrix);

    _float4x4 ParticleWorld{};
    XMStoreFloat4x4(&ParticleWorld, matWorld);

    return ParticleWorld;
}
