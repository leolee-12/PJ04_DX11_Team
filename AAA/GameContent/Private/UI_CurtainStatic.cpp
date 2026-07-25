#include "UI_CurtainStatic.h"
#include "Shader.h"
#include "Texture.h"

CUI_CurtainStatic::CUI_CurtainStatic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_CurtainAnimBase{ pDevice, pContext }, m_vColor{ 1.f, 1.f, 1.f, 1.f } {
}
CUI_CurtainStatic::CUI_CurtainStatic(const CUI_CurtainStatic& Prototype)
    : CUI_CurtainAnimBase(Prototype), m_vColor{ Prototype.m_vColor } {
}

HRESULT CUI_CurtainStatic::Initialize(void* pArg)
{
    if (pArg)
        m_vColor = static_cast<UI_CURTAINSTATIC_DESC*>(pArg)->vColor;

    if (FAILED(__super::Initialize(pArg)))   // super가 fStartSize로 균일 스케일 세팅하지만,
        return E_FAIL;                        // 이후 Deserialize의 Set_WorldMatrix가 에디터 트랜스폼으로 덮어씀

    // 트랜스폼은 안 건드림 -> 에디터에서 조절한 비균일 XY 스케일이 그대로 유지
    m_bPlay = false; m_bLoop = false; m_bArmed = false; m_bFinished = false;
    return S_OK;
}

void CUI_CurtainStatic::Update(_float fTimeDelta)
{
    CUIPartObject::Update(fTimeDelta);
}

HRESULT CUI_CurtainStatic::Bind_Material(CShader* pShader)
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

CUI_CurtainStatic* CUI_CurtainStatic::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_CurtainStatic* p = new CUI_CurtainStatic(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CUI_CurtainStatic"); Safe_Release(p); }
    return p;
}
CGameObject* CUI_CurtainStatic::Clone(void* pArg)
{
    CUI_CurtainStatic* p = new CUI_CurtainStatic(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CUI_CurtainStatic"); Safe_Release(p); }
    return p;
}
void CUI_CurtainStatic::Free() { __super::Free(); }