#include "Effect_Quad.h"

#include "GameInstance.h"

CEffect_Quad::CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Quad::CEffect_Quad(const CEffect_Quad& Prototype)
    : CEffect_Part(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Quad::Initialize_Prototype()
{
    return S_OK;
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

void CEffect_Quad::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_Quad::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEffect_Quad::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_Quad::Render()
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

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Quad::Bind_ShaderValue()
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

void CEffect_Quad::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);

    Update_TexSpriteAnimation(fTimeDelta, fRatio);
    Update_MaskSpriteAnimation(fTimeDelta, fRatio);
}

void CEffect_Quad::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_UVScroll(fTimeDelta, fRatio);
}

void CEffect_Quad::Update_EffectPart(const _float fTimeDelta, const _float fRatio)
{
}

void CEffect_Quad::Update_TexSpriteAnimation(const _float fTimeDelta, const _float fRatio)
{
    if (m_bSpriteAniTexture == true)
    {
        _int iTotalCount = m_iTexFrameX * m_iTexFrameY;
        _int iCurTexFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);

        if (iCurTexFrameIndex >= iTotalCount)
            iCurTexFrameIndex -= 1;

        _float fCurTexFrameX = static_cast<_float>(iCurTexFrameIndex % m_iTexFrameX);
        _float fCurTexFrameY = static_cast<_float>(iCurTexFrameIndex / m_iTexFrameX);

        m_fCurTexAniSize.x = 1.f / static_cast<_float>(m_iTexFrameX);
        m_fCurTexAniSize.y = 1.f / static_cast<_float>(m_iTexFrameY);

        m_fCurTexAniUV.x = m_fCurTexAniSize.x * fCurTexFrameX;
        m_fCurTexAniUV.y = m_fCurTexAniSize.y * fCurTexFrameY;
    }
}

void CEffect_Quad::Update_MaskSpriteAnimation(const _float fTimeDelta, const _float fRatio)
{
    if (m_bSpriteAniMask == true)
    {
        _int iTotalCount = m_iMaskFrameX * m_iMaskFrameY;
        _int iCurMaskFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);

        if (iCurMaskFrameIndex >= iTotalCount)
            iCurMaskFrameIndex -= 1;

        _float fCurMaskFrameX = static_cast<_float>(iCurMaskFrameIndex % m_iMaskFrameX);
        _float fCurMaskFrameY = static_cast<_float>(iCurMaskFrameIndex / m_iMaskFrameX);

        m_fCurMaskAniSize.x = 1.f / static_cast<_float>(m_iMaskFrameX);
        m_fCurMaskAniSize.y = 1.f / static_cast<_float>(m_iMaskFrameY);

        m_fCurMaskAniUV.x = m_fCurMaskAniSize.x * fCurMaskFrameX;
        m_fCurMaskAniUV.y = m_fCurMaskAniSize.y * fCurMaskFrameY;
    }
}

void CEffect_Quad::Init_PropertyValue()
{
    m_bSpriteAniTexture = false;
    m_iTexFrameX = 1;
    m_iTexFrameY = 1;

    m_bSpriteAniMask = false;
    m_iMaskFrameX = 1;
    m_iMaskFrameY = 1;
}

void CEffect_Quad::Free()
{   
    __super::Free();
}