#include "Effect_Emitter.h"

#include "GameInstance.h"

CEffect_Emitter::CEffect_Emitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Emitter::CEffect_Emitter(const CEffect_Emitter& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Emitter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Emitter::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    Reset_Emitter();

    return S_OK;
}

void CEffect_Emitter::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_Emitter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bActive == false && m_bEmitterWasActive == true)
    {
        Reset_Emitter();
        m_bEmitterWasActive = false;
    }
}

void CEffect_Emitter::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    Compute_CombinedWorldMatrix();
}

HRESULT CEffect_Emitter::Render()
{
    return S_OK;
}

void CEffect_Emitter::Effect_Start()
{
    __super::Effect_Start();

    Reset_Emitter();
    m_fEmitterPreviousRatio = 0.f;
    m_bEmitterWasActive = false;
}

HRESULT CEffect_Emitter::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Emitter::Ready_Components()
{
    return S_OK;
}

void CEffect_Emitter::Init_PropertyValue()
{
    // Emission
    m_iEmitterMaxParticleCount = 100;
    m_fEmitterRateOverTime = 20.f;
    m_iEmitterBurstCount = 0;

    // Lifetime
    m_fEmitterLifeTime = 2.f;
    m_bEmitterRandomLifeTime = false;
    m_vEmitterLifeTimeRange = { 1.f, 2.f };

    // Shape
    m_iEmitterShapeType = EMITTER_SHAPE_POINT;

    m_fEmitterShapeRadius = 1.f;
    m_bEmitterShapeRandomRadius = false;

    m_vEmitterBoxSize = { 5.f, 3.f, 5.f };
    m_iEmitterBoxSpawnMode = EMITTER_BOX_TOP;

    // Velocity
    m_iEmitterVelocityMode = EMITTER_VELOCITY_SHAPE_OUTWARD;
    m_fEmitterStartSpeed = 3.f;

    m_vEmitterVelocityDirection = { 0.f, -1.f, 0.f };

    m_fEmitterFountainSpread = 1.f;
    m_fEmitterFountainUpBias = 1.5f;

    m_fEmitterDirectionRandomStrength = 0.f;
    m_fEmitterStartSpeedRandomRatio = 0.f;

    // Force
    m_bEmitterUseAcceleration = false;
    m_vEmitterAcceleration = { 0.f, 0.f, 0.f };

    // Flutter
    m_bEmitterUseFlutter = false;
    m_fEmitterFlutterAmplitude = 0.3f;
    m_fEmitterFlutterFrequency = 1.5f;
    m_fEmitterFlutterRandomRatio = 0.3f;

    // Alpha
    m_fEmitterAlpha = 1.f;

    m_bEmitterAlphaChange = false;
    m_fEmitterAlphaStartValue = 1.f;
    m_fEmitterAlphaEndValue = 0.f;

    m_bActive_EmitterAlpha_Ratio_0 = false;
    m_fEmitterAlpha_Ratio_0 = 0.5f;
    m_fEmitterAlpha_Value_0 = 1.f;

    m_bActive_EmitterAlpha_Ratio_1 = false;
    m_fEmitterAlpha_Ratio_1 = 0.75f;
    m_fEmitterAlpha_Value_1 = 1.f;

    // Size
    m_fEmitterStartSize = 0.5f;

    m_bEmitterRandomSize = false;
    m_vEmitterStartSizeRange = { 0.3f, 0.7f };

    m_bEmitterSizeChange = false;
    m_fEmitterSizeStartValue = 1.f;
    m_fEmitterSizeEndValue = 1.f;

    m_bActive_EmitterSize_Ratio_0 = false;
    m_fEmitterSize_Ratio_0 = 0.5f;
    m_fEmitterSize_Value_0 = 1.f;

    m_bActive_EmitterSize_Ratio_1 = false;
    m_fEmitterSize_Ratio_1 = 0.75f;
    m_fEmitterSize_Value_1 = 1.f;

    // Color
    m_vEmitterColor = { 1.f, 1.f, 1.f };

    m_bEmitterColorChange = false;
    m_vEmitterColorStartValue = { 1.f, 1.f, 1.f };
    m_vEmitterColorEndValue = { 1.f, 1.f, 1.f };

    m_bActive_EmitterColor_Ratio_0 = false;
    m_fEmitterColor_Ratio_0 = 0.5f;
    m_vEmitterColor_Value_0 = { 1.f, 1.f, 1.f };

    m_bActive_EmitterColor_Ratio_1 = false;
    m_fEmitterColor_Ratio_1 = 0.75f;
    m_vEmitterColor_Value_1 = { 1.f, 1.f, 1.f };

    // Rotation
    m_bEmitterRandomRotation = false;
    m_vEmitterRandomRotationMin = { 0.f, 0.f, 0.f };
    m_vEmitterRandomRotationMax = { 0.f, 360.f, 0.f };

    m_bEmitterRotationOverLife = false;
    m_vEmitterAngularVelocity = { 0.f, 0.f, 0.f };
}

void CEffect_Emitter::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    const _bool bRestart = m_bEmitterWasActive == false ||
        fRatio + Helper::fEpsilon < m_fEmitterPreviousRatio;

    if (bRestart == true)
    {
        Reset_Emitter();
        Emit_Particles(m_iEmitterBurstCount);
    }

    m_bEmitterWasActive = true;
    m_fEmitterPreviousRatio = fRatio;
    m_fEmitterElapsedTime = fRatio * Get_PartDuration();

    __super::Update_Core(fTimeDelta, fRatio);

    if (Can_Emit() == true)
        Emit_ByRate(fTimeDelta);

    Update_EmitterParticles(fTimeDelta);
}

void CEffect_Emitter::Reset_Emitter()
{
    if (m_iEmitterMaxParticleCount < 1)
        m_iEmitterMaxParticleCount = 1;

    m_EmitterParticles.clear();
    m_EmitterParticles.resize(m_iEmitterMaxParticleCount);

    m_fEmitterElapsedTime = 0.f;
    m_fEmitterSpawnAccumulator = 0.f;
}

void CEffect_Emitter::Update_EmitterParticles(_float fTimeDelta)
{
    for (EMITTER_PARTICLE& Particle : m_EmitterParticles)
    {
        if (Particle.bAlive == false)
            continue;

        Particle.fAge += fTimeDelta;

        if (Particle.fLifeTime <= Helper::fEpsilon || Particle.fAge >= Particle.fLifeTime)
        {
            Particle.bAlive = false;
            continue;
        }

        _float fLocalRatio = Particle.fAge / Particle.fLifeTime;
        Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

        Update_EmitterParticleMove(Particle);

        if (m_bEmitterRotationOverLife == true)
        {
            Particle.vRotation.x += Particle.vAngularVelocity.x * fTimeDelta;
            Particle.vRotation.y += Particle.vAngularVelocity.y * fTimeDelta;
            Particle.vRotation.z += Particle.vAngularVelocity.z * fTimeDelta;
        }

        Update_EmitterParticleAlpha(Particle, fLocalRatio);
        Update_EmitterParticleSize(Particle, fLocalRatio);
        Update_EmitterParticleColor(Particle, fLocalRatio);
    }
}

void CEffect_Emitter::Emit_ByRate(_float fTimeDelta)
{
    if (m_fEmitterRateOverTime <= 0.f)
        return;

    m_fEmitterSpawnAccumulator += m_fEmitterRateOverTime * fTimeDelta;

    _uint iEmitCount = static_cast<_uint>(m_fEmitterSpawnAccumulator);
    if (iEmitCount == 0)
        return;

    m_fEmitterSpawnAccumulator -= static_cast<_float>(iEmitCount);

    Emit_Particles(iEmitCount);
}

void CEffect_Emitter::Emit_Particles(_uint iEmitCount)
{
    for (_uint i = 0; i < iEmitCount; ++i)
    {
        if (Spawn_EmitterParticle() == false)
            break;
    }
}

_bool CEffect_Emitter::Spawn_EmitterParticle()
{
    EMITTER_PARTICLE* pParticle = Find_DeadParticle();
    if (pParticle == nullptr)
        return false;

    pParticle->bAlive = true;
    pParticle->fAge = 0.f;

    _float fLifeTime = m_fEmitterLifeTime;

    if (m_bEmitterRandomLifeTime == true)
    {
        _float fMinLifeTime = m_vEmitterLifeTimeRange.x;
        _float fMaxLifeTime = m_vEmitterLifeTimeRange.y;

        if (fMaxLifeTime < fMinLifeTime)
            std::swap(fMinLifeTime, fMaxLifeTime);

        if (fMinLifeTime < Helper::fEpsilon)
            fMinLifeTime = Helper::fEpsilon;

        if (fMaxLifeTime < Helper::fEpsilon)
            fMaxLifeTime = Helper::fEpsilon;

        fLifeTime = m_pGameInstance_Proxy->RandomFloat(fMinLifeTime, fMaxLifeTime);
    }

    if (fLifeTime < Helper::fEpsilon)
        fLifeTime = Helper::fEpsilon;

    pParticle->fLifeTime = fLifeTime;

    Helper::IntClamp(m_iEmitterShapeType, EMITTER_SHAPE_POINT, EMITTER_SHAPE_END - 1);
    Helper::IntClamp(m_iEmitterVelocityMode, EMITTER_VELOCITY_SHAPE_OUTWARD, EMITTER_VELOCITY_END - 1);

    pParticle->vSpawnLocalPos = Make_EmitterSpawnLocalPos();
    pParticle->vLocalPos = pParticle->vSpawnLocalPos;

    _vector vDir = Make_EmitterVelocityDirection(*pParticle);
    vDir = Apply_EmitterVelocityRandom(vDir);

    const _float fSpeed = Make_EmitterRandomSpeed();

    XMStoreFloat3(&pParticle->vVelocity, vDir * fSpeed);

    _float fParticleSize = m_fEmitterStartSize;

    if (m_bEmitterRandomSize == true)
    {
        _float fMinSize = m_vEmitterStartSizeRange.x;
        _float fMaxSize = m_vEmitterStartSizeRange.y;

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

    pParticle->fBaseSize = fParticleSize;
    pParticle->vScale = { fParticleSize, fParticleSize, fParticleSize };

    pParticle->fAlpha = m_fEmitterAlpha;
    pParticle->vColor = m_vEmitterColor;

    pParticle->fFlutterPhase = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

    _float fRandomRatio = m_fEmitterFlutterRandomRatio;
    Helper::FloatClamp(fRandomRatio, 0.f, 1.f);

    const _float fFlutterAmpMin = m_fEmitterFlutterAmplitude * (1.f - fRandomRatio);
    const _float fFlutterAmpMax = m_fEmitterFlutterAmplitude * (1.f + fRandomRatio);
    pParticle->fFlutterAmplitude = m_pGameInstance_Proxy->RandomFloat(fFlutterAmpMin, fFlutterAmpMax);

    const _float fFlutterFreqMin = m_fEmitterFlutterFrequency * (1.f - fRandomRatio);
    const _float fFlutterFreqMax = m_fEmitterFlutterFrequency * (1.f + fRandomRatio);
    pParticle->fFlutterFrequency = m_pGameInstance_Proxy->RandomFloat(fFlutterFreqMin, fFlutterFreqMax);

    const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
    pParticle->vFlutterDir = { cosf(fTheta), 0.f, sinf(fTheta) };

    pParticle->vRotation = { 0.f, 0.f, 0.f };

    if (m_bEmitterRandomRotation == true)
    {
        _float3 vMin = m_vEmitterRandomRotationMin;
        _float3 vMax = m_vEmitterRandomRotationMax;

        if (vMax.x < vMin.x)
            std::swap(vMin.x, vMax.x);
        if (vMax.y < vMin.y)
            std::swap(vMin.y, vMax.y);
        if (vMax.z < vMin.z)
            std::swap(vMin.z, vMax.z);

        pParticle->vRotation.x = m_pGameInstance_Proxy->RandomFloat(vMin.x, vMax.x);
        pParticle->vRotation.y = m_pGameInstance_Proxy->RandomFloat(vMin.y, vMax.y);
        pParticle->vRotation.z = m_pGameInstance_Proxy->RandomFloat(vMin.z, vMax.z);
    }

    pParticle->vAngularVelocity = m_vEmitterAngularVelocity;

    return true;
}

CEffect_Emitter::EMITTER_PARTICLE* CEffect_Emitter::Find_DeadParticle()
{
    for (EMITTER_PARTICLE& Particle : m_EmitterParticles)
    {
        if (Particle.bAlive == false)
            return &Particle;
    }

    return nullptr;
}

_float3 CEffect_Emitter::Make_EmitterSpawnLocalPos() const
{
    _int iShapeType = m_iEmitterShapeType;
    Helper::IntClamp(iShapeType, EMITTER_SHAPE_POINT, EMITTER_SHAPE_END - 1);

    switch (iShapeType)
    {
    case EMITTER_SHAPE_SPHERE:
    {
        _float fRadius = m_fEmitterShapeRadius;

        if (fRadius < 0.f)
            fRadius = 0.f;

        if (m_bEmitterShapeRandomRadius == true)
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

    case EMITTER_SHAPE_CIRCLE:
    {
        _float fRadius = m_fEmitterShapeRadius;

        if (fRadius < 0.f)
            fRadius = 0.f;

        if (m_bEmitterShapeRandomRadius == true)
            fRadius = m_pGameInstance_Proxy->RandomFloat(0.f, fRadius);

        const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

        return {
            m_fPivot.x + cosf(fTheta) * fRadius,
            m_fPivot.y,
            m_fPivot.z + sinf(fTheta) * fRadius
        };
    }

    case EMITTER_SHAPE_BOX:
    {
        const _float fHalfX = fabsf(m_vEmitterBoxSize.x) * 0.5f;
        const _float fHalfY = fabsf(m_vEmitterBoxSize.y) * 0.5f;
        const _float fHalfZ = fabsf(m_vEmitterBoxSize.z) * 0.5f;

        _int iBoxSpawnMode = m_iEmitterBoxSpawnMode;
        Helper::IntClamp(iBoxSpawnMode, EMITTER_BOX_VOLUME, EMITTER_BOX_END - 1);

        _float3 vSpawnPos{};
        vSpawnPos.x = m_fPivot.x + m_pGameInstance_Proxy->RandomFloat(-fHalfX, fHalfX);
        vSpawnPos.z = m_fPivot.z + m_pGameInstance_Proxy->RandomFloat(-fHalfZ, fHalfZ);

        if (iBoxSpawnMode == EMITTER_BOX_TOP)
            vSpawnPos.y = m_fPivot.y + fHalfY;
        else
            vSpawnPos.y = m_fPivot.y + m_pGameInstance_Proxy->RandomFloat(-fHalfY, fHalfY);

        return vSpawnPos;
    }

    case EMITTER_SHAPE_POINT:
    default:
        return m_fPivot;
    }
}

_vector CEffect_Emitter::Make_EmitterVelocityDirection(const EMITTER_PARTICLE& Particle) const
{
    _int iVelocityMode = m_iEmitterVelocityMode;
    Helper::IntClamp(iVelocityMode, EMITTER_VELOCITY_SHAPE_OUTWARD, EMITTER_VELOCITY_END - 1);

    switch (iVelocityMode)
    {
    case EMITTER_VELOCITY_DIRECTION:
    {
        _vector vDir = XMLoadFloat3(&m_vEmitterVelocityDirection);

        if (XMVectorGetX(XMVector3LengthSq(vDir)) <= Helper::fEpsilon)
            vDir = XMVectorSet(0.f, -1.f, 0.f, 0.f);

        return XMVector3Normalize(vDir);
    }

    case EMITTER_VELOCITY_FOUNTAIN:
        return Make_FountainDirection();

    case EMITTER_VELOCITY_SHAPE_OUTWARD:
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

_vector CEffect_Emitter::Apply_EmitterVelocityRandom(_vector vBaseDir) const
{
    _float fRandomStrength = m_fEmitterDirectionRandomStrength;
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

_float CEffect_Emitter::Make_EmitterRandomSpeed() const
{
    _float fRandomRatio = m_fEmitterStartSpeedRandomRatio;
    Helper::FloatClamp(fRandomRatio, 0.f, 1.f);

    if (fRandomRatio <= Helper::fEpsilon)
        return m_fEmitterStartSpeed;

    _float fMinSpeed = m_fEmitterStartSpeed * (1.f - fRandomRatio);
    _float fMaxSpeed = m_fEmitterStartSpeed * (1.f + fRandomRatio);

    if (fMaxSpeed < fMinSpeed)
        std::swap(fMinSpeed, fMaxSpeed);

    return m_pGameInstance_Proxy->RandomFloat(fMinSpeed, fMaxSpeed);
}

_vector CEffect_Emitter::Make_RandomSphereDirection() const
{
    const _float fY = m_pGameInstance_Proxy->RandomFloat(-1.f, 1.f);
    const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

    const _float fRadius = sqrtf(max(0.f, 1.f - fY * fY));
    const _float fX = cosf(fTheta) * fRadius;
    const _float fZ = sinf(fTheta) * fRadius;

    return XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));
}

_vector CEffect_Emitter::Make_FountainDirection() const
{
    _float fSpread = m_fEmitterFountainSpread;
    _float fUpBias = m_fEmitterFountainUpBias;

    Helper::FloatClamp(fSpread, 0.f, 10.f);

    if (fUpBias < Helper::fEpsilon)
        fUpBias = Helper::fEpsilon;

    const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
    const _float fHorizontal = m_pGameInstance_Proxy->RandomFloat(0.f, fSpread);

    const _float fX = cosf(fTheta) * fHorizontal;
    const _float fZ = sinf(fTheta) * fHorizontal;

    return XMVector3Normalize(XMVectorSet(fX, fUpBias, fZ, 0.f));
}

void CEffect_Emitter::Update_EmitterParticleMove(EMITTER_PARTICLE& Particle)
{
    const _float fTime = Particle.fAge;

    _float3 vAcceleration{};

    if (m_bEmitterUseAcceleration == true)
        vAcceleration = m_vEmitterAcceleration;

    Particle.vLocalPos.x =
        Particle.vSpawnLocalPos.x +
        Particle.vVelocity.x * fTime +
        0.5f * vAcceleration.x * fTime * fTime;

    Particle.vLocalPos.y =
        Particle.vSpawnLocalPos.y +
        Particle.vVelocity.y * fTime +
        0.5f * vAcceleration.y * fTime * fTime;

    Particle.vLocalPos.z =
        Particle.vSpawnLocalPos.z +
        Particle.vVelocity.z * fTime +
        0.5f * vAcceleration.z * fTime * fTime;

    if (m_bEmitterUseFlutter == true)
    {
        const _float fFlutter =
            (sinf(fTime * XM_2PI * Particle.fFlutterFrequency + Particle.fFlutterPhase) -
                sinf(Particle.fFlutterPhase)) *
            Particle.fFlutterAmplitude;

        Particle.vLocalPos.x += Particle.vFlutterDir.x * fFlutter;
        Particle.vLocalPos.z += Particle.vFlutterDir.z * fFlutter;
    }
}

void CEffect_Emitter::Update_EmitterParticleAlpha(EMITTER_PARTICLE& Particle, _float fLocalRatio)
{
    _float fAlpha = Evaluate_EmitterFloatCurve(
        fLocalRatio, m_fEmitterAlpha, m_bEmitterAlphaChange,
        m_fEmitterAlphaStartValue, m_fEmitterAlphaEndValue,
        m_bActive_EmitterAlpha_Ratio_0, m_fEmitterAlpha_Ratio_0, m_fEmitterAlpha_Value_0,
        m_bActive_EmitterAlpha_Ratio_1, m_fEmitterAlpha_Ratio_1, m_fEmitterAlpha_Value_1);

    Helper::FloatClamp(fAlpha, 0.f, 1.f);

    Particle.fAlpha = fAlpha;
}

void CEffect_Emitter::Update_EmitterParticleSize(EMITTER_PARTICLE& Particle, _float fLocalRatio)
{
    _float fSizeRatio = Evaluate_EmitterFloatCurve(
        fLocalRatio, 1.f, m_bEmitterSizeChange,
        m_fEmitterSizeStartValue, m_fEmitterSizeEndValue,
        m_bActive_EmitterSize_Ratio_0, m_fEmitterSize_Ratio_0, m_fEmitterSize_Value_0,
        m_bActive_EmitterSize_Ratio_1, m_fEmitterSize_Ratio_1, m_fEmitterSize_Value_1);

    _float fSize = Particle.fBaseSize * fSizeRatio;

    if (fSize < Helper::fEpsilon)
        fSize = Helper::fEpsilon;

    Particle.vScale = { fSize, fSize, fSize };
}

void CEffect_Emitter::Update_EmitterParticleColor(EMITTER_PARTICLE& Particle, _float fLocalRatio)
{
    Particle.vColor = Evaluate_EmitterFloat3Curve(
        fLocalRatio, m_vEmitterColor, m_bEmitterColorChange,
        m_vEmitterColorStartValue, m_vEmitterColorEndValue,
        m_bActive_EmitterColor_Ratio_0, m_fEmitterColor_Ratio_0, m_vEmitterColor_Value_0,
        m_bActive_EmitterColor_Ratio_1, m_fEmitterColor_Ratio_1, m_vEmitterColor_Value_1);
}

_bool CEffect_Emitter::Can_Emit() const
{
    const _float fPartDuration = Get_PartDuration();
    if (fPartDuration <= Helper::fEpsilon)
        return false;

    return m_fEmitterElapsedTime <= fPartDuration;
}

_bool CEffect_Emitter::Has_AliveParticle() const
{
    for (const EMITTER_PARTICLE& Particle : m_EmitterParticles)
    {
        if (Particle.bAlive == true)
            return true;
    }

    return false;
}

_float CEffect_Emitter::Evaluate_EmitterFloatCurve(_float fLocalRatio, _float fFixedValue, _bool bChange, _float
    fStartValue, _float fEndValue, _bool bActiveRatio0, _float fRatio0, _float fValue0, _bool bActiveRatio1, _float
    fRatio1, _float fValue1) const
{
    if (bChange == false)
        return fFixedValue;

    RATIO_VALUE Points[4]{};
    _uint iCount = 0;

    Points[iCount++] = { 0.f, fStartValue };

    if (bActiveRatio0 == true)
        Points[iCount++] = { fRatio0, fValue0 };

    if (bActiveRatio1 == true)
        Points[iCount++] = { fRatio1, fValue1 };

    Points[iCount++] = { 1.f, fEndValue };

    for (_uint i = 0; i < iCount; ++i)
        Helper::FloatClamp(Points[i].fRatio, 0.f, 1.f);

    for (_uint i = 0; i < iCount; ++i)
    {
        for (_uint j = i + 1; j < iCount; ++j)
        {
            if (Points[j].fRatio < Points[i].fRatio)
                std::swap(Points[i], Points[j]);
        }
    }

    Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

    for (_uint i = 0; i + 1 < iCount; ++i)
    {
        if (fLocalRatio <= Points[i + 1].fRatio)
        {
            const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;

            if (fRange <= Helper::fEpsilon)
                return Points[i + 1].fValue;

            const _float fStep = Helper::FloatSmoothStep(
                Points[i].fRatio,
                Points[i + 1].fRatio,
                fLocalRatio);

            return Points[i].fValue + (Points[i + 1].fValue - Points[i].fValue) * fStep;
        }
    }

    return Points[iCount - 1].fValue;
}

_float3 CEffect_Emitter::Evaluate_EmitterFloat3Curve(_float fLocalRatio, const _float3& vFixedValue, _bool bChange,
    const _float3& vStartValue, const _float3& vEndValue, _bool bActiveRatio0, _float fRatio0, const _float3& vValue0,
    _bool bActiveRatio1, _float fRatio1, const _float3& vValue1) const
{
    if (bChange == false)
        return vFixedValue;

    RATIO_VALUE_FLOAT3 Points[4]{};
    _uint iCount = 0;

    Points[iCount++] = { 0.f, vStartValue };

    if (bActiveRatio0 == true)
        Points[iCount++] = { fRatio0, vValue0 };

    if (bActiveRatio1 == true)
        Points[iCount++] = { fRatio1, vValue1 };

    Points[iCount++] = { 1.f, vEndValue };

    for (_uint i = 0; i < iCount; ++i)
        Helper::FloatClamp(Points[i].fRatio, 0.f, 1.f);

    for (_uint i = 0; i < iCount; ++i)
    {
        for (_uint j = i + 1; j < iCount; ++j)
        {
            if (Points[j].fRatio < Points[i].fRatio)
                std::swap(Points[i], Points[j]);
        }
    }

    Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

    for (_uint i = 0; i + 1 < iCount; ++i)
    {
        if (fLocalRatio <= Points[i + 1].fRatio)
        {
            const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;

            if (fRange <= Helper::fEpsilon)
                return Points[i + 1].vValue;

            const _float fStep = Helper::FloatSmoothStep(
                Points[i].fRatio,
                Points[i + 1].fRatio,
                fLocalRatio);

            _float3 vResult{};
            vResult.x = Points[i].vValue.x + (Points[i + 1].vValue.x - Points[i].vValue.x) * fStep;
            vResult.y = Points[i].vValue.y + (Points[i + 1].vValue.y - Points[i].vValue.y) * fStep;
            vResult.z = Points[i].vValue.z + (Points[i + 1].vValue.z - Points[i].vValue.z) * fStep;

            return vResult;
        }
    }

    return Points[iCount - 1].vValue;
}

_float4x4 CEffect_Emitter::Make_EmitterParticleWorldMatrix(const EMITTER_PARTICLE& Particle) const
{
    _matrix matScale = XMMatrixScaling(
        Particle.vScale.x,
        Particle.vScale.y,
        Particle.vScale.z);

    _matrix matRotation =
        XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(Particle.vRotation.x),
            XMConvertToRadians(Particle.vRotation.y),
            XMConvertToRadians(Particle.vRotation.z));

    _matrix matTranslation = XMMatrixTranslation(
        Particle.vLocalPos.x,
        Particle.vLocalPos.y,
        Particle.vLocalPos.z);

    _matrix matWorld =
        matScale *
        matRotation *
        matTranslation *
        XMLoadFloat4x4(&m_CombinedWorldMatrix);

    _float4x4 ParticleWorld{};
    XMStoreFloat4x4(&ParticleWorld, matWorld);

    return ParticleWorld;
}

void CEffect_Emitter::Free()
{
    __super::Free();
}