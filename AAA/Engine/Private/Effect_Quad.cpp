#include "Effect_Quad.h"

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

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bBillboard", &m_bBillboard, sizeof(m_bBillboard))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRoll", &m_fRoll, sizeof(m_fRoll))))
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

void CEffect_Quad::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    if (m_bSpriteAniTexture == true)
        Evaluate_SpriteFrame(m_iTexFrameX, m_iTexFrameY, fRatio, m_fCurTexAniUV, m_fCurTexAniSize);

    if (m_bSpriteAniMask == true)
        Evaluate_SpriteFrame(m_iMaskFrameX, m_iMaskFrameY, fRatio, m_fCurMaskAniUV, m_fCurMaskAniSize);
}

void CEffect_Quad::Init_PropertyValue()
{
    m_bBillboard = false;

    m_bSpriteAniTexture = false;
    m_iTexFrameX = 1;
    m_iTexFrameY = 1;

    m_bSpriteAniMask = false;
    m_iMaskFrameX = 1;
    m_iMaskFrameY = 1;
}
