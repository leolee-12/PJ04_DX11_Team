#include "UI_Eraser.h"
#include "Shader.h"
#include "Texture.h"

CUI_Eraser::CUI_Eraser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainAnimBase{ pDevice, pContext } {
}
CUI_Eraser::CUI_Eraser(const CUI_Eraser& Prototype)
    : CUI_CurtainAnimBase(Prototype) {
}

HRESULT CUI_Eraser::Bind_Material(CShader* pShader)
{
    if (!m_pTextureCom) return E_FAIL;      // 마스크 필수
    if (FAILED(m_pTextureCom->Bind_ShaderResource(pShader, "g_Texture", m_iTexIndex))) return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha)))) return E_FAIL;     // 마스크*g_fAlpha
    return S_OK;
}

CUI_Eraser* CUI_Eraser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Eraser* p = new CUI_Eraser(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_Eraser"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_Eraser::Clone(void* pArg)
{
    CUI_Eraser* p = new CUI_Eraser(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_Eraser"); Safe_Release(p); }
    return p;
}
void CUI_Eraser::Free() { __super::Free(); }