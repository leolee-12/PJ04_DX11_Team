#include "Star2DParticle.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CStar2DParticle::CStar2DParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_RectParticle{ pDevice, pContext }
{
    Init_PropertyValue();
}

CStar2DParticle::CStar2DParticle(const CStar2DParticle& Prototype)
    : CEffect_RectParticle(Prototype)
{
    Init_PropertyValue();
}

HRESULT CStar2DParticle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CStar2DParticle::Initialize(void* pArg)
{
    m_iParticleCount = STAR_COUNT;
    m_bParticleSpawnRandom = false;
    m_fParticleSpawnStartRatio = 0.f;
    m_fParticleSpawnEndRatio = 0.f;
    m_fParticleLifeRatio = 1.f;

    EFFECT_RECTPARTICLE_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_Star2D.iLevelID;
    tDesc.wstrTextureTag = Texture_Star2D.szProtoTag;

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";


    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CStar2DParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CStar2DParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CStar2DParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND_HDR, this);
}

HRESULT CStar2DParticle::Render()
{
    __super::Render();

    return S_OK;
}

void CStar2DParticle::On_Deserialized()
{
    m_iParticleCount = STAR_COUNT;
    __super::On_Deserialized();
}

void CStar2DParticle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    _float fSpawnIntervalRatio = m_fStarSpawnIntervalRatio;
    Helper::FloatClamp(fSpawnIntervalRatio, 0.f, 1.f);

    _float fLeaderTravelRatio = m_fStarLeaderTravelRatio;
    Helper::FloatClamp(fLeaderTravelRatio, Helper::fEpsilon, 1.f);

    _float fLeaderLifeRatio = m_fStarLeaderLifeRatio;
    Helper::FloatClamp(fLeaderLifeRatio, Helper::fEpsilon, 1.f);

    if (fLeaderTravelRatio > fLeaderLifeRatio)
        fLeaderTravelRatio = fLeaderLifeRatio;

    _float fTrailLifeRatio = m_fStarTrailLifeRatio;
    Helper::FloatClamp(fTrailLifeRatio, Helper::fEpsilon, 1.f);

    const _float fRadius = fabsf(m_fStarArcRadius);
    const _float fStartAngle = XMConvertToRadians(m_fStarStartAngle);
    const _float fStartX = cosf(fStartAngle) * fRadius;
    const _float fStartY = sinf(fStartAngle) * fRadius;
    const _float fDirectionSign = m_fStarArcDegrees < 0.f ? -1.f : 1.f;

    for (_uint i = 0; i < m_Particles.size(); ++i)
    {
        PARTICLE& Particle = m_Particles[i];

        if (i >= STAR_COUNT)
        {
            Particle.bAlive = false;
            continue;
        }

        _float fLocalRatio = 0.f;

        if (i == 0)
        {
            if (fRatio > fLeaderLifeRatio)
            {
                Particle.bAlive = false;
                continue;
            }

            fLocalRatio = fRatio / fLeaderLifeRatio;
            Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

            _float fMoveRatio = fRatio / fLeaderTravelRatio;
            Helper::FloatClamp(fMoveRatio, 0.f, 1.f);

            const _float fArcRatio =
                Helper::FloatSmoothStep(0.f, 1.f, fMoveRatio);
            const _float fAngle = XMConvertToRadians(
                m_fStarStartAngle + m_fStarArcDegrees * fArcRatio);

            Particle.vLocalPos.x = m_fPivot.x + cosf(fAngle) * fRadius - fStartX;
            Particle.vLocalPos.y = m_fPivot.y + sinf(fAngle) * fRadius - fStartY;
            Particle.vLocalPos.z = m_fPivot.z;
        }
        else
        {
            const _float fSpawnRatio = static_cast<_float>(i) * fSpawnIntervalRatio;
            const _float fAgeRatio = fRatio - fSpawnRatio;

            if (fAgeRatio < 0.f || fAgeRatio > fTrailLifeRatio)
            {
                Particle.bAlive = false;
                continue;
            }

            fLocalRatio = fAgeRatio / fTrailLifeRatio;
            Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

            _float fLeaderRatioAtSpawn = fSpawnRatio / fLeaderTravelRatio;
            Helper::FloatClamp(fLeaderRatioAtSpawn, 0.f, 1.f);

            const _float fSpawnArcRatio =
                Helper::FloatSmoothStep(0.f, 1.f, fLeaderRatioAtSpawn);
            const _float fSpawnAngle = XMConvertToRadians(
                m_fStarStartAngle + m_fStarArcDegrees * fSpawnArcRatio);

            const _float fRemainRatio = 1.f - fLocalRatio;
            const _float fDriftRatio = 1.f -
                fRemainRatio * fRemainRatio * fRemainRatio;
            const _float fDriftDistance =
                m_fStarTrailDriftDistance * fDriftRatio;

            const _float fTangentX = -sinf(fSpawnAngle) * fDirectionSign;
            const _float fTangentY = cosf(fSpawnAngle) * fDirectionSign;

            Particle.vLocalPos.x =
                m_fPivot.x + cosf(fSpawnAngle) * fRadius - fStartX +
                fTangentX * fDriftDistance;
            Particle.vLocalPos.y =
                m_fPivot.y + sinf(fSpawnAngle) * fRadius - fStartY +
                fTangentY * fDriftDistance;
            Particle.vLocalPos.z = m_fPivot.z;
        }

        Particle.bAlive = true;
        Update_ParticleAlpha(Particle, fLocalRatio);
        Update_ParticleSize(Particle, fLocalRatio);
        Update_ParticleColor(Particle, fLocalRatio);

        _float fSizeScale = i == 0
            ? m_fStarLeaderSizeScale
            : m_fStarTrailSizeScale;
        if (fSizeScale < 0.f)
            fSizeScale = 0.f;

        Particle.vScale.x *= fSizeScale;
        Particle.vScale.y *= fSizeScale;
        Particle.vScale.z *= fSizeScale;
    }
}

void CStar2DParticle::Init_PropertyValue()
{
    m_fStarArcRadius = 1.f;
    m_fStarStartAngle = 0.f;
    m_fStarArcDegrees = 30.f;
    m_fStarSpawnIntervalRatio = 0.1f;
    m_fStarLeaderTravelRatio = 0.6f;
    m_fStarLeaderLifeRatio = 0.7f;
    m_fStarTrailLifeRatio = 0.55f;
    m_fStarTrailDriftDistance = 0.08f;
    m_fStarLeaderSizeScale = 1.f;
    m_fStarTrailSizeScale = 0.8f;
}

CStar2DParticle* CStar2DParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStar2DParticle* pInstance = new CStar2DParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CStar2DParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CStar2DParticle::Clone(void* pArg)
{
    CStar2DParticle* pInstance = new CStar2DParticle(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CStar2DParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStar2DParticle::Free()
{
    __super::Free();
}
