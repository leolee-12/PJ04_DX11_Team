#include "SpinWind.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CSpinWind::CSpinWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
    , m_fSpawnHeightMin (0.f) 
    , m_fSpawnHeightMax (2.5f)
{
}

CSpinWind::CSpinWind(const CSpinWind& Prototype)
    : CEffect_MeshParticle(Prototype)
    , m_fSpawnHeightMin(Prototype.m_fSpawnHeightMin)
    , m_fSpawnHeightMax(Prototype.m_fSpawnHeightMax)
{
}

HRESULT CSpinWind::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSpinWind::Initialize(void* pArg)
{
    EFFECT_MESHPARTICLE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = MODEL_PROTO_TAG;

    // wind 메쉬(TornadoPieceMedium)는 UNKNOWN 슬롯 Smoke.png 1장뿐
    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;    // Smoke 텍스처를 g_UnknownTexture로
    tDesc.bUseNormalTexture = false;
    tDesc.bUseMRATexture = false;

    // 기본 셰이더(Shader_Effect_Mesh) 사용 - 커스텀 셰이더 불필요
    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    // ----- 가산(Additive) 패스 -----
    m_iShaderPass = ShaderPass::Additive;   // 2

    // ----- 파티클 파라미터(베이스로 처리, 에디터에서 튜닝) -----
    m_iParticleCount = 8;

    // 보스 둘레 링에서 스폰
    m_iParticleShapeType = PARTICLE_SHAPE_CIRCLE;
    m_fParticleShapeRadius = 2.0f;
    m_bParticleShapeRandomRadius = true;

    // 위로 솟구치며 퍼짐(분수)
    m_iParticleVelocityMode = PARTICLE_VELOCITY_FOUNTAIN;
    m_fParticleStartSpeed = 3.0f;
    m_fParticleStartSpeedRandomRatio = 0.3f;
    m_fParticleFountainSpread = 0.5f;
    m_fParticleFountainUpBias = 1.5f;

    // 스핀 느낌: 수명 동안 Y축 회전
    m_bParticleRandomRotation = true;
    m_vParticleRandomRotationMin = { 0.f,   0.f, 0.f };
    m_vParticleRandomRotationMax = { 0.f, 360.f, 0.f };
    m_bParticleRotationOverLife = true;
    m_vParticleAngularVelocity = { 0.f, 720.f, 0.f };

    // 스폰/수명 분산
    m_bParticleSpawnRandom = true;
    m_fParticleSpawnStartRatio = 0.f;
    m_fParticleSpawnEndRatio = 0.3f;
    m_fParticleLifeRatio = 0.6f;

    // 크기: 살짝 커지며
    m_fParticleStartSize = 1.f;
    m_bParticleRandomSize = true;
    m_vParticleStartSizeRange = { 0.8f, 1.3f };
    m_bParticleSizeChange = true;
    m_fParticleSizeStartValue = 0.7f;
    m_fParticleSizeEndValue = 1.3f;

    // 색/알파: 하늘빛 틴트, 페이드 인->아웃(가산이라 0으로)
    m_vParticleColor = { 0.7f, 0.85f, 1.0f };
    m_fParticleAlpha = 1.f;
    m_bParticleAlphaChange = true;
    m_fParticleAlphaStartValue = 0.f;
    m_fParticleAlphaEndValue = 0.f;
    m_bActive_ParticleAlpha_Ratio_0 = true;
    m_fParticleAlpha_Ratio_0 = 0.25f;   // 25% 지점에서 최대
    m_fParticleAlpha_Value_0 = 1.f;

    return S_OK;
}

void CSpinWind::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CSpinWind::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CSpinWind::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);   // 록과 달리 투명 그룹
}

HRESULT CSpinWind::Render()
{
    __super::Render();
    return S_OK;
}

void CSpinWind::Effect_Start()
{
    __super::Effect_Start();

    for (auto& P : m_Particles)
    {
        const _float fY = m_pGameInstance_Proxy->RandomFloat(m_fSpawnHeightMin, m_fSpawnHeightMax);
        P.vSpawnLocalPos.y += fY;   
        P.vLocalPos.y = P.vSpawnLocalPos.y;   
    }
}

CSpinWind* CSpinWind::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSpinWind* pInstance = new CSpinWind(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSpinWind");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CSpinWind::Clone(void* pArg)
{
    CSpinWind* pInstance = new CSpinWind(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSpinWind");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSpinWind::Free() { __super::Free(); }