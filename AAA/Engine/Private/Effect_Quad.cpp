#include "Effect_Quad.h"

#include "Effect_RectCommon.h"
#include "GameInstance.h"

CEffect_Quad::CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_NonParticle(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Quad::CEffect_Quad(const CEffect_Quad& Prototype)
    : CEffect_NonParticle(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Quad::Initialize(void* pArg)
{
    EFFECT_QUAD_DESC* pDesc = static_cast<EFFECT_QUAD_DESC*>(pArg);

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

HRESULT CEffect_Quad::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    const _int iPass = Resolve_ShaderPass();
    if (FAILED(m_pShaderCom->Begin(iPass)))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Quad::Ready_Components()
{
    if(m_bCustomShader == false)
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

HRESULT CEffect_Quad::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(Bind_ViewProjectionMatrices()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Quad::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    if (m_bCustomShader == false &&
        FAILED(EffectRect::Bind_GBufferOutput(m_pShaderCom, Get_UseGBufferOutput())))
        return E_FAIL;

    auto Values = Make_RectValues();
    return EffectRect::Bind_ShaderValues(m_pShaderCom, Values, m_fRoll);
}

void CEffect_Quad::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    auto Values = Make_RectValues();
    EffectRect::Update_SpriteAnimations(Values, fRatio);
}

void CEffect_Quad::Init_PropertyValue()
{
    auto Values = Make_RectValues();
    EffectRect::Initialize_DefaultValues(Values);
}

EffectRect::VALUES CEffect_Quad::Make_RectValues()
{
    return {
        m_bBillboard,
        m_bSpriteAniTexture, m_iTexFrameX, m_iTexFrameY, m_fCurTexAniUV, m_fCurTexAniSize,
        m_bSpriteAniMask, m_iMaskFrameX, m_iMaskFrameY, m_fCurMaskAniUV, m_fCurMaskAniSize
    };
}
