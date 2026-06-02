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

    const _uint iAlphaBlendPass = 1;
    if (FAILED(m_pShaderCom->Begin(iAlphaBlendPass)))
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
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
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

    return S_OK;
}

void CEffect_Quad::Update_EffectPart(const _float fTimeDelta, const _float fActiveTime, const _float fRatio)
{
}

void CEffect_Quad::Update_UVScroll(const _float fTimeDelta)
{
    __super::Update_UVScroll(fTimeDelta);
}

void CEffect_Quad::Init_PropertyValue()
{

}

void CEffect_Quad::Free()
{   
    __super::Free();
}