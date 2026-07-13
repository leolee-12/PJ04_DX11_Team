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

HRESULT CEffect_RectEmitter::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    const _int iPass = Resolve_ShaderPass();

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

        _float fRoll = XMConvertToRadians(Particle.vRotation.z);
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fRoll", &fRoll, sizeof(fRoll))))
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
    if (FAILED(Bind_ViewProjectionMatrices()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_RectEmitter::Bind_ShaderValue(_float fLocalRatio)
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bBillboard", &m_bBillboard, sizeof(m_bBillboard))))
        return E_FAIL;

    Evaluate_SpriteFrame(
        m_iTexFrameX, m_iTexFrameY,
        m_bSpriteAniTexture == true ? fLocalRatio : 0.f,
        m_fCurTexAniUV, m_fCurTexAniSize);
    Evaluate_SpriteFrame(
        m_iMaskFrameX, m_iMaskFrameY,
        m_bSpriteAniMask == true ? fLocalRatio : 0.f,
        m_fCurMaskAniUV, m_fCurMaskAniSize);

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
