#include "RockFloorEffect.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CRockFloorEffect::CRockFloorEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CRockFloorEffect::CRockFloorEffect(const CRockFloorEffect& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CRockFloorEffect::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CRockFloorEffect::Initialize(void* pArg)
{
    EFFECT_MESHPARTICLE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = MODEL_PROTO_TAG;

    // 바닥균열 모델: 디퓨즈 + 노멀 + MRA
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

    // ----- 파티클 파라미터: 바닥 판이 랜덤하게 기울며 들어올려짐 (출발값) -----
    // 바닥 평면에 여러 판 분포 (Box-Top)
    m_iParticleCount = 8;
    m_iParticleShapeType = PARTICLE_SHAPE_BOX;
    m_iParticleBoxSpawnMode = PARTICLE_BOX_TOP;
    m_vParticleBoxSize = { 5.f, 0.f, 5.f };

    // 천천히 들어올려짐 (살짝 상승, 떨어지지 않음)
    m_iParticleVelocityMode = PARTICLE_VELOCITY_DIRECTION;
    m_vParticleVelocityDirection = { 0.f, 1.f, 0.f };
    m_fParticleStartSpeed = 1.2f;
    m_fParticleStartSpeedRandomRatio = 0.5f;
    m_bParticleUseAcceleration = false;

    // 랜덤하게 기울어진 채 (바닥판이라 X/Z 기울기 위주, Y는 방향만)
    m_bParticleRandomRotation = true;
    m_vParticleRandomRotationMin = { -25.f, -180.f, -25.f };
    m_vParticleRandomRotationMax = { 25.f,  180.f,  25.f };

    // 판마다 들리고 사라지는 시점 제각각
    m_bParticleSpawnRandom = true;
    m_fParticleSpawnStartRatio = 0.f;
    m_fParticleSpawnEndRatio = 0.5f;
    m_fParticleLifeRatio = 0.6f;

    // 크기 약간 랜덤
    m_fParticleStartSize = 1.f;
    m_bParticleRandomSize = true;
    m_vParticleStartSizeRange = { 0.8f, 1.3f };

    // 수명 끝에 알파 1 -> 0 : 셰이더 디더가 이 알파로 페이드아웃
    m_fParticleAlpha = 1.f;
    m_bParticleAlphaChange = true;
    m_fParticleAlphaStartValue = 1.f;
    m_fParticleAlphaEndValue = 0.f;
    m_bActive_ParticleAlpha_Ratio_0 = true;
    m_fParticleAlpha_Ratio_0 = 0.65f;   // 0~65% 구간 불투명 유지
    m_fParticleAlpha_Value_0 = 1.f;

    return S_OK;
}

void CRockFloorEffect::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CRockFloorEffect::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CRockFloorEffect::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CRockFloorEffect::Render()
{
    __super::Render();
    return S_OK;
}

CRockFloorEffect* CRockFloorEffect::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
    CRockFloorEffect* pInstance = new CRockFloorEffect(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRockFloorEffect");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRockFloorEffect::Clone(void* pArg)
{
    CRockFloorEffect* pInstance = new CRockFloorEffect(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRockFloorEffect");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRockFloorEffect::Free() { __super::Free(); }