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

HRESULT CEffect_Particle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Particle::Initialize(void* pArg)
{
    EFFECT_PARTICLE_DESC* pDesc = static_cast<EFFECT_PARTICLE_DESC*>(pArg);

    m_iVIBufferLevel = pDesc->iVIBufferLevel;
    m_wstrVIBufferTag = pDesc->wstrVIBufferTag;

    m_bCustomShader = pDesc->bCustomShader;
    m_iShaderLevel = pDesc->iShaderLevel;
    m_wstrShaderTag = pDesc->wstrShaderTag;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    Reset_Particles();

    return S_OK;
}

void CEffect_Particle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_Particle::Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    if (m_fDuration <= 0.f)
        return;

    _float fRatio = m_fAccTime / m_fDuration;
    Helper::FloatClamp(fRatio, 0.f, 1.f);

    Update_Particles_ByContainerTime(fRatio);
}

void CEffect_Particle::Late_Update(_float fTimeDelta)
{
    Compute_CombinedWorldMatrix();
}

HRESULT CEffect_Particle::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    Helper::IntClamp(m_iShaderPass, ShaderPass::Default, ShaderPass::ShaderPass_End - 1);
    if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    for (const PARTICLE& Particle : m_Particles)
    {
        if (Particle.bAlive == false)
            continue;

        _float4x4 ParticleWorld = Make_ParticleWorldMatrix(Particle);

        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &ParticleWorld)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
            return E_FAIL;

        if (FAILED(m_pVIBuffer->Render()))
            return E_FAIL;
    }

    return S_OK;
}

void CEffect_Particle::Effect_Start()
{
    __super::Effect_Start();

    Reset_Particles();
}

HRESULT CEffect_Particle::Ready_Components()
{
    if (m_bCustomShader == false)
        m_pShaderCom = m_pGameInstance_Proxy->Get_2DShader();
    else
        m_pShaderCom = Add_Component<CShader>(m_iShaderLevel, m_wstrShaderTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pVIBuffer = Add_Component<CVIBuffer_Rect>(m_iVIBufferLevel, m_wstrVIBufferTag, TEXT("Com_Buffer"));
    if (m_pVIBuffer == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Particle::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Particle::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bSpriteAniTexture", &m_bSpriteAniTexture, sizeof(m_bSpriteAniTexture))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniTexUV", &m_fCurTexAniUV, sizeof(m_fCurTexAniUV))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniTexSize", &m_fCurTexAniSize, sizeof(m_fCurTexAniSize))))
        return E_FAIL;


    if (FAILED(m_pShaderCom->Bind_RawValue("g_bSpriteAniMask", &m_bSpriteAniMask, sizeof(m_bSpriteAniMask))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniMaskUV", &m_fCurMaskAniUV, sizeof(m_fCurMaskAniUV))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniMaskSize", &m_fCurMaskAniSize, sizeof(m_fCurMaskAniSize))))
        return E_FAIL;

    return S_OK;
}

void CEffect_Particle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
}

void CEffect_Particle::Init_PropertyValue()
{
    m_bSpriteAniTexture = false;
    m_iTexFrameX = 1;
    m_iTexFrameY = 1;

    m_bSpriteAniMask = false;
    m_iMaskFrameX = 1;
    m_iMaskFrameY = 1;
}

void CEffect_Particle::Reset_Particles()
{
    m_iParticleCount = PARTICLE_MAX_COUNT;

    m_Particles.clear();
    m_Particles.resize(m_iParticleCount);

    for (_uint i = 0; i < m_iParticleCount; ++i)
    {
        PARTICLE& Particle = m_Particles[i];

        const _float fAngle = XM_2PI * static_cast<_float>(i) / static_cast<_float>(m_iParticleCount);

        // XZ 평면 기준으로 바깥으로 퍼짐.
        _vector vDir = XMVectorSet(cosf(fAngle), 0.f, sinf(fAngle), 0.f);
        vDir = XMVector3Normalize(vDir);

        Particle.bAlive = true;
        Particle.fAge = 0.f;
        Particle.fLifeTime = m_fParticleLifeTime;

        Particle.vLocalPos = m_fPivot;

        XMStoreFloat3(&Particle.vVelocity, vDir * m_fParticleSpeed);

        Particle.vScale = { m_fParticleSize, m_fParticleSize, m_fParticleSize };
    }
}

_float4x4 CEffect_Particle::Make_ParticleWorldMatrix(const PARTICLE& Particle) const
{
    _matrix matScale = XMMatrixScaling(Particle.vScale.x, Particle.vScale.y, Particle.vScale.z);

    _matrix matTranslation = XMMatrixTranslation(Particle.vLocalPos.x, Particle.vLocalPos.y, Particle.vLocalPos.z);

    _matrix matWorld = matScale * matTranslation * XMLoadFloat4x4(&m_CombinedWorldMatrix);

    _float4x4 ParticleWorld{};
    XMStoreFloat4x4(&ParticleWorld, matWorld);

    return ParticleWorld;
}

void CEffect_Particle::Update_Particles_ByContainerTime(_float fRatio)
{
    for (PARTICLE& Particle : m_Particles)
    {
        if (fRatio >= 1.f)
        {
            Particle.bAlive = false;
            continue;
        }

        Particle.bAlive = true;

        const _float fMoveTime = fRatio *
            Particle.fLifeTime;

        Particle.vLocalPos.x = m_fPivot.x +
            Particle.vVelocity.x * fMoveTime;
        Particle.vLocalPos.y = m_fPivot.y +
            Particle.vVelocity.y * fMoveTime;
        Particle.vLocalPos.z = m_fPivot.z +
            Particle.vVelocity.z * fMoveTime;
    }
}

void CEffect_Particle::Free()
{
    __super::Free();
}