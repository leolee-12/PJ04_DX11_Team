#include "Effect_RectEmitter.h"

#include "Effect_RectCommon.h"
#include "GameInstance.h"
#include "Shader.h"

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
    if (FAILED(Bind_ViewProjectionMatrices()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    const _int iPass = Resolve_ShaderPass();

    for (const EMITTER_PARTICLE& Particle : m_EmitterParticles)
    {
        if (Particle.bAlive == false)
            continue;

        const _float4x4 ParticleWorld = Make_EmitterParticleWorldMatrix(Particle);
        if (FAILED(EffectRect::Bind_ParticleDrawValues(
            m_pShaderCom, ParticleWorld, Particle.fAlpha, Particle.vColor)))
            return E_FAIL;

        if (FAILED(Bind_EmitterRectValue(Particle)))
            return E_FAIL;

        if (FAILED(EffectRect::Begin_AndRender(m_pShaderCom, m_pVIBuffer, iPass)))
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

HRESULT CEffect_RectEmitter::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    if (m_bCustomShader == false &&
        FAILED(EffectRect::Bind_GBufferOutput(m_pShaderCom, Get_UseGBufferOutput())))
        return E_FAIL;

    auto Values = Make_RectValues();
    EffectRect::Update_SpriteAnimations(Values, 0.f, true);
    const _bool bUseTextureFrame = m_bSpriteAniTexture || m_bRandomTextureFrame;

    if (FAILED(EffectRect::Bind_StaticShaderValues(m_pShaderCom, Values)) ||
        FAILED(EffectRect::Bind_SpriteShaderValues(m_pShaderCom, Values)) ||
        FAILED(m_pShaderCom->Bind_RawValue("g_bSpriteAniTexture", &bUseTextureFrame, sizeof(_bool))) ||
        FAILED(EffectRect::Bind_Roll(m_pShaderCom, 0.f)))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_RectEmitter::Bind_EmitterRectValue(const EMITTER_PARTICLE& Particle)
{
    auto Values = Make_RectValues();

    if (m_bSpriteAniTexture == true || m_bRandomTextureFrame == true || m_bSpriteAniMask == true)
    {
        _float fParticleRatio = 1.f;
        if (Particle.fLifeTime > Helper::fEpsilon)
            fParticleRatio = Particle.fAge / Particle.fLifeTime;
        Helper::FloatClamp(fParticleRatio, 0.f, 1.f);

        EffectRect::Update_SpriteAnimations(Values, fParticleRatio);

        if (m_bRandomTextureFrame == true)
        {
            _int iFrameX = m_iTexFrameX;
            _int iFrameY = m_iTexFrameY;
            if (iFrameX < 1)
                iFrameX = 1;
            if (iFrameY < 1)
                iFrameY = 1;

            const _int iTotalFrames = iFrameX * iFrameY;
            _int iFrameIndex = static_cast<_int>(Particle.fRandomValue * static_cast<_float>(iTotalFrames));
            Helper::IntClamp(iFrameIndex, 0, iTotalFrames - 1);

            const _float fFrameRatio =
                (static_cast<_float>(iFrameIndex) + 0.5f) /
                static_cast<_float>(iTotalFrames);
            Evaluate_SpriteFrame(
                iFrameX, iFrameY, fFrameRatio,
                Values.vCurrentTexUV, Values.vCurrentTexSize);
        }

        if (FAILED(EffectRect::Bind_SpriteShaderValues(m_pShaderCom, Values)))
            return E_FAIL;
    }

    if (m_bUseParticleRoll == true)
    {
        const _float fRoll = XMConvertToRadians(Particle.vRotation.z);
        if (FAILED(EffectRect::Bind_Roll(m_pShaderCom, fRoll)))
            return E_FAIL;
    }

    return S_OK;
}

void CEffect_RectEmitter::Init_PropertyValue()
{
    auto Values = Make_RectValues();
    EffectRect::Initialize_DefaultValues(Values);
    m_bUseParticleRoll = true;
    m_bRandomTextureFrame = false;
}

EffectRect::VALUES CEffect_RectEmitter::Make_RectValues()
{
    return {
        m_bBillboard,
        m_bTextureColorToAlpha,
        {
            m_bUseTextureUVEdgeFade,
            m_iTextureUVEdgeFadeAxis,
            m_fTextureUVEdgeFadeStartRange,
            m_fTextureUVEdgeFadeEndRange,
            m_fTextureUVEdgeFadePower
        },
        m_bSpriteAniTexture, m_iTexFrameX, m_iTexFrameY, m_fCurTexAniUV, m_fCurTexAniSize,
        m_bSpriteAniMask, m_iMaskFrameX, m_iMaskFrameY, m_fCurMaskAniUV, m_fCurMaskAniSize
    };
}
