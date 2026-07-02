#include "Effect_RectEmitter.h"

#include "GameInstance.h"

CEffect_RectEmitter::CEffect_RectEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Emitter(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_RectEmitter::CEffect_RectEmitter(const CEffect_RectEmitter& Prototype)
    : CEffect_Emitter(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_RectEmitter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_RectEmitter::Initialize(void* pArg)
{
    EFFECT_RECTEMITTER_DESC* pDesc = static_cast<EFFECT_RECTEMITTER_DESC*>(pArg);

    m_iVIBufferLevel = pDesc->iVIBufferLevel;
    m_wstrVIBufferTag = pDesc->wstrVIBufferTag;

    m_bCustomShader = pDesc->bCustomShader;
    m_iShaderLevel = pDesc->iShaderLevel;
    m_wstrShaderTag = pDesc->wstrShaderTag;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CEffect_RectEmitter::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_RectEmitter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEffect_RectEmitter::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_RectEmitter::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    Helper::IntClamp(m_iShaderPass, ShaderPass::Default, ShaderPass::ShaderPass_End - 1);
    Helper::IntClamp(m_iMirror, Sampler::DEFAULT, Sampler::SAMPLER_END - 1);
    Helper::IntClamp(m_iDepthIgnore, DepthMode::DEPTH_DEFAULT, DepthMode::DEPTH_MODE_END - 1);

    _int iPass = m_iShaderPass +
        (m_iMirror == Sampler::MIRROR ? ShaderPass::ShaderPass_End : 0) +
        (m_iDepthIgnore == DepthMode::DEPTH_IGNORE ? ShaderPass::ShaderPass_End * Sampler::SAMPLER_END : 0);

    for (const EMITTER_PARTICLE& Particle : m_EmitterParticles)
    {
        if (Particle.bAlive == false)
            continue;

        _float fLocalRatio = 1.f;
        if (Particle.fLifeTime > Helper::fEpsilon)
            fLocalRatio = Particle.fAge / Particle.fLifeTime;

        Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

        _float4x4 ParticleWorld = Make_EmitterParticleWorldMatrix(Particle);

        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &ParticleWorld)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &Particle.fAlpha, sizeof(Particle.fAlpha))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &Particle.vColor, sizeof(Particle.vColor))))
            return E_FAIL;

        if (FAILED(Bind_ShaderValue(fLocalRatio)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;

        if (FAILED(m_pVIBuffer->Render()))
            return E_FAIL;
    }

    return S_OK;
}

void CEffect_RectEmitter::Effect_Start()
{
    __super::Effect_Start();
}

HRESULT CEffect_RectEmitter::Ready_Components()
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

HRESULT CEffect_RectEmitter::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_RectEmitter::Bind_ShaderValue(_float fLocalRatio)
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bBillboard", &m_bBillboard, sizeof(m_bBillboard))))
        return E_FAIL;

    Update_TexSpriteAnimation(fLocalRatio);
    Update_MaskSpriteAnimation(fLocalRatio);

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

void CEffect_RectEmitter::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);
}

void CEffect_RectEmitter::Update_TexSpriteAnimation(_float fRatio)
{
    _int iFrameX = m_iTexFrameX;
    _int iFrameY = m_iTexFrameY;

    if (iFrameX < 1)
        iFrameX = 1;

    if (iFrameY < 1)
        iFrameY = 1;

    m_fCurTexAniSize.x = 1.f / static_cast<_float>(iFrameX);
    m_fCurTexAniSize.y = 1.f / static_cast<_float>(iFrameY);

    m_fCurTexAniUV = { 0.f, 0.f };

    if (m_bSpriteAniTexture == false)
        return;

    Helper::FloatClamp(fRatio, 0.f, 1.f);

    _int iTotalCount = iFrameX * iFrameY;
    _int iCurTexFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);

    if (iCurTexFrameIndex >= iTotalCount)
        iCurTexFrameIndex = iTotalCount - 1;

    _float fCurTexFrameX = static_cast<_float>(iCurTexFrameIndex % iFrameX);
    _float fCurTexFrameY = static_cast<_float>(iCurTexFrameIndex / iFrameX);

    m_fCurTexAniUV.x = m_fCurTexAniSize.x * fCurTexFrameX;
    m_fCurTexAniUV.y = m_fCurTexAniSize.y * fCurTexFrameY;
}

void CEffect_RectEmitter::Update_MaskSpriteAnimation(_float fRatio)
{
    _int iFrameX = m_iMaskFrameX;
    _int iFrameY = m_iMaskFrameY;

    if (iFrameX < 1)
        iFrameX = 1;

    if (iFrameY < 1)
        iFrameY = 1;

    m_fCurMaskAniSize.x = 1.f / static_cast<_float>(iFrameX);
    m_fCurMaskAniSize.y = 1.f / static_cast<_float>(iFrameY);

    m_fCurMaskAniUV = { 0.f, 0.f };

    if (m_bSpriteAniMask == false)
        return;

    Helper::FloatClamp(fRatio, 0.f, 1.f);

    _int iTotalCount = iFrameX * iFrameY;
    _int iCurMaskFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);

    if (iCurMaskFrameIndex >= iTotalCount)
        iCurMaskFrameIndex = iTotalCount - 1;

    _float fCurMaskFrameX = static_cast<_float>(iCurMaskFrameIndex % iFrameX);
    _float fCurMaskFrameY = static_cast<_float>(iCurMaskFrameIndex / iFrameX);

    m_fCurMaskAniUV.x = m_fCurMaskAniSize.x * fCurMaskFrameX;
    m_fCurMaskAniUV.y = m_fCurMaskAniSize.y * fCurMaskFrameY;
}

void CEffect_RectEmitter::Init_PropertyValue()
{
    m_bBillboard = false;

    m_bSpriteAniTexture = false;
    m_iTexFrameX = 1;
    m_iTexFrameY = 1;

    m_bSpriteAniMask = false;
    m_iMaskFrameX = 1;
    m_iMaskFrameY = 1;
}

void CEffect_RectEmitter::Free()
{
    __super::Free();
}
