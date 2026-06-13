#include "UI_CurtainTexture.h"
#include "Shader.h"
#include "Texture.h"

CUI_CurtainTexture::CUI_CurtainTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainAnimBase{ pDevice, pContext }, m_vColor{ 1.f, 1.f, 1.f, 1.f } {
}
CUI_CurtainTexture::CUI_CurtainTexture(const CUI_CurtainTexture& Prototype)
    : CUI_CurtainAnimBase(Prototype), m_vColor{ Prototype.m_vColor } {
}

HRESULT CUI_CurtainTexture::Initialize(void* pArg)
{
    if (pArg)
        m_vColor = static_cast<UI_CURTAINTEXTURE_DESC*>(pArg)->vColor;
    return __super::Initialize(pArg);       // 텍스처/애니/위치 공통 처리
}

HRESULT CUI_CurtainTexture::Bind_Material(CShader* pShader)
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

CUI_CurtainTexture* CUI_CurtainTexture::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_CurtainTexture* p = new CUI_CurtainTexture(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_CurtainTexture"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_CurtainTexture::Clone(void* pArg)
{
    CUI_CurtainTexture* p = new CUI_CurtainTexture(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_CurtainTexture"); Safe_Release(p); }
    return p;
}
void CUI_CurtainTexture::Free() { __super::Free(); }