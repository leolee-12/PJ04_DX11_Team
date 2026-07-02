#include "Swing_Smoke.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CSwing_Smoke::CSwing_Smoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
    , m_fRadius(3.0f)
    , m_fStartAngleDeg(0.f)
    , m_fArcSpanDeg(180.f)
    , m_fSweepWindow(0.35f)
    , m_fFaceOffsetDeg(0.f)
{
}

CSwing_Smoke::CSwing_Smoke(const CSwing_Smoke& Prototype)
    : CEffect_MeshParticle(Prototype)
    , m_fRadius(Prototype.m_fRadius)
    , m_fStartAngleDeg(Prototype.m_fStartAngleDeg)
    , m_fArcSpanDeg(Prototype.m_fArcSpanDeg)
    , m_fSweepWindow(Prototype.m_fSweepWindow)
    , m_fFaceOffsetDeg(Prototype.m_fFaceOffsetDeg)
{
}

HRESULT CSwing_Smoke::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSwing_Smoke::Initialize(void* pArg)
{
    EFFECT_MESHPARTICLE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = MODEL_PROTO_TAG;

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = false;
    tDesc.bUseNormalTexture = false;
    tDesc.bUseMRATexture = false;

    // 기본 셰이더의 GBUFFER(불투명) 패스
    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    m_iShaderPass = ShaderPass::Default;   // 0 = GBUFFER 불투명

    
    m_fRadius = 3.0f;
    m_fStartAngleDeg = 0.f;   
    m_fArcSpanDeg = 180.f;   
    m_fSweepWindow = 0.35f;   
    m_fFaceOffsetDeg = 0.f;

    // ----- 파티클 기본 -----
    m_iParticleCount = 12;      // 호를 따라 놓일 개수
    m_bParticleSpawnRandom = false;   // 순차라 랜덤 끔(아래서 덮어씀)
    m_fParticleLifeRatio = 0.4f;    // 주의: SweepWindow + LifeRatio <= 1

    m_iParticleVelocityMode = PARTICLE_VELOCITY_DIRECTION;
    m_fParticleStartSpeed = 0.f;

    m_fParticleStartSize = 1.f;
    m_bParticleRandomSize = true;
    m_vParticleStartSizeRange = { 0.8f, 1.2f };

    m_fParticleAlpha = 1.f;

    return S_OK;
}

void CSwing_Smoke::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CSwing_Smoke::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CSwing_Smoke::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);   // 불투명
}

HRESULT CSwing_Smoke::Render()
{
    __super::Render();
    return S_OK;
}

void CSwing_Smoke::Effect_Start()
{
    __super::Effect_Start(); 

    const _uint N = (_uint)m_Particles.size();
    for (_uint i = 0; i < N; ++i)
    {
        PARTICLE& P = m_Particles[i];

        const _float t = (N > 1) ? (_float)i / (_float)(N - 1) : 0.f;

        const _float fDeg = m_fStartAngleDeg + t * m_fArcSpanDeg;
        const _float fRad = XMConvertToRadians(fDeg);

        P.vSpawnLocalPos = { cosf(fRad) * m_fRadius, 0.f, sinf(fRad) * m_fRadius };
        P.vLocalPos = P.vSpawnLocalPos;

        P.fStartRatio = t * m_fSweepWindow;
        P.fEndRatio = P.fStartRatio + m_fParticleLifeRatio;

        P.vBaseRotation.y += fDeg + m_fFaceOffsetDeg;
        P.vRotation = P.vBaseRotation;
    }
}

CSwing_Smoke* CSwing_Smoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSwing_Smoke* pInstance = new CSwing_Smoke(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSwing_Smoke");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CSwing_Smoke::Clone(void* pArg)
{
    CSwing_Smoke* pInstance = new CSwing_Smoke(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSwing_Smoke");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSwing_Smoke::Free() { __super::Free(); }