#include "UI_Curtain.h"
#include "Shader.h"
#include "Texture.h"

CUI_Curtain::CUI_Curtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainAnimBase{ pDevice, pContext }, m_vColor{ 1.f, 1.f, 1.f, 1.f } {
}
CUI_Curtain::CUI_Curtain(const CUI_Curtain& Prototype)
    : CUI_CurtainAnimBase(Prototype), m_vColor{ Prototype.m_vColor } {
}

HRESULT CUI_Curtain::Initialize(void* pArg)
{
    if (pArg)
        m_vColor = static_cast<UI_CURTAIN_DESC*>(pArg)->vColor;
    return __super::Initialize(pArg);
}

HRESULT CUI_Curtain::Bind_Material(CShader* pShader)
{
    _bool bUseTex = (m_pTextureCom != nullptr);
    if (bUseTex)
        if (FAILED(m_pTextureCom->Bind_ShaderResource(pShader, "g_Texture", m_iTexIndex))) return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_bUseTexture", &bUseTex, sizeof(_bool)))) return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor)))) return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha)))) return E_FAIL;
    _float4 vUV = { 1.f, 1.f, 0.f, 0.f };
    if (FAILED(pShader->Bind_RawValue("g_vUVTransform", &vUV, sizeof(_float4)))) return E_FAIL;
    return S_OK;
}

CUI_Curtain* CUI_Curtain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Curtain* p = new CUI_Curtain(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_Curtain"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_Curtain::Clone(void* pArg)
{
    CUI_Curtain* p = new CUI_Curtain(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_Curtain"); Safe_Release(p); }
    return p;
}
void CUI_Curtain::Free() { __super::Free(); }