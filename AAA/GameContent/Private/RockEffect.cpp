#include "RockEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CRockEffect::CRockEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CRockEffect::CRockEffect(const CRockEffect& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CRockEffect::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CRockEffect::Initialize(void* pArg)
{
    EFFECT_MESHPARTICLE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = MODEL_PROTO_TAG;

    // 돌 모델: 디퓨즈 + 노멀 + MRA
    tDesc.bUseDiffuseTexture = true;
    tDesc.bUseUnKnownTexture = false;
    tDesc.bUseNormalTexture = true;   // 1단계(b)에서 추가한 필드
    tDesc.bUseMRATexture = true;

    // Rock 전용 디더 셰이더
    tDesc.bCustomShader = true;
    tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
    tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    // ----- 파티클 파라미터(출발값, 에디터/플레이 보면서 튜닝) -----
    // 바닥 평면에 흩뿌림 (Box-Top)
    m_iParticleCount = 12;
    m_iParticleShapeType = PARTICLE_SHAPE_BOX;
    m_iParticleBoxSpawnMode = PARTICLE_BOX_TOP;
    m_vParticleBoxSize = { 6.f, 0.f, 6.f };

    // 위로 들어올려지는 느낌
    m_iParticleVelocityMode = PARTICLE_VELOCITY_DIRECTION;
    m_vParticleVelocityDirection = { 0.f, 1.f, 0.f };
    m_fParticleStartSpeed = 3.f;
    m_fParticleStartSpeedRandomRatio = 0.4f;

    // 솟았다 살짝 가라앉음
    m_bParticleUseAcceleration = true;
    m_vParticleAcceleration = { 0.f, -6.f, 0.f };

    // 랜덤하게 기울어진 채
    m_bParticleRandomRotation = true;
    m_vParticleRandomRotationMin = { -40.f,   0.f, -40.f };
    m_vParticleRandomRotationMax = { 40.f, 360.f,  40.f };

    // 메쉬마다 등장/소멸 시점 제각각
    m_bParticleSpawnRandom = true;
    m_fParticleSpawnStartRatio = 0.f;
    m_fParticleSpawnEndRatio = 0.4f;
    m_fParticleLifeRatio = 0.5f;

    // 크기 약간 랜덤
    m_fParticleStartSize = 1.f;
    m_bParticleRandomSize = true;
    m_vParticleStartSizeRange = { 0.6f, 1.4f };

    // 수명 끝에 알파 1 -> 0 : 셰이더 디더가 이 알파로 페이드아웃
    m_fParticleAlpha = 1.f;
    m_bParticleAlphaChange = true;
    m_fParticleAlphaStartValue = 1.f;
    m_fParticleAlphaEndValue = 0.f;
    m_bActive_ParticleAlpha_Ratio_0 = true;
    m_fParticleAlpha_Ratio_0 = 0.6f;   // 0~60% 구간은 불투명 유지
    m_fParticleAlpha_Value_0 = 1.f;

    return S_OK;
}

void CRockEffect::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CRockEffect::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CRockEffect::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CRockEffect::Render()
{
    __super::Render();
    return S_OK;
}

CRockEffect* CRockEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRockEffect* pInstance = new CRockEffect(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRockEffect");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRockEffect::Clone(void* pArg)
{
    CRockEffect* pInstance = new CRockEffect(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRockEffect");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRockEffect::Free() { __super::Free(); }