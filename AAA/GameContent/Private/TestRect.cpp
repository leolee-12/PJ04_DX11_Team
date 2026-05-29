#include "TestRect.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "Navigation.h"

CTestRect::CTestRect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
    , m_bFlipX(false)
    , m_bFlipY(false)
    , m_vColor{ 1.f, 1.f, 1.f }
    , m_fAlpha(1.f)
    , m_bAlphaTest(false)
    , m_fTestAlpha(0.f)
    , m_fUVCutRight(1.f)
    , m_fUVCutLeft(0.f)
    , m_fUVCutTop(0.f)
    , m_fUVCutBottom(1.f)
{

}

CTestRect::CTestRect(const CTestRect& Prototype)
    : CGameObject(Prototype)
    , m_bFlipX(false)
    , m_bFlipY(false)
    , m_vColor{ 1.f, 1.f, 1.f }
    , m_fAlpha(1.f)
    , m_bAlphaTest(false)
    , m_fTestAlpha(0.f)
    , m_fUVCutRight(0.f)
    , m_fUVCutLeft(0.f)
    , m_fUVCutTop(0.f)
    , m_fUVCutBottom(0.f)
{

}

HRESULT CTestRect::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestRect::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CTestRect::Priority_Update(_float fTimeDelta)
{
}

void CTestRect::Update(_float fTimeDelta)
{
}

void CTestRect::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CTestRect::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(ETOUI(VTXTEX_SHADER::ALPHABLEND))))
        return E_FAIL;
   
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CTestRect::Bind_ShaderValue()
{
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bFlipX", &m_bFlipX, sizeof(m_bFlipX))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bFlipY", &m_bFlipY, sizeof(m_bFlipY))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bAlphaTest", &m_bAlphaTest, sizeof(m_bAlphaTest))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTestAlpha", &m_fTestAlpha, sizeof(m_fTestAlpha))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fUVCutRight", &m_fUVCutRight, sizeof(m_fUVCutRight))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fUVCutLeft", &m_fUVCutLeft, sizeof(m_fUVCutLeft))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fUVCutTop", &m_fUVCutTop, sizeof(m_fUVCutTop))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fUVCutBottom", &m_fUVCutBottom, sizeof(m_fUVCutBottom))))
        return E_FAIL;
}

HRESULT CTestRect::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_VtxTex.iLevelID, Shader_VtxTex.szProtoTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pVIBuffer = Add_Component<CVIBuffer_Rect>(VI_Rect.iLevelID, VI_Rect.szProtoTag, TEXT("Com_Model"));
    if (m_pVIBuffer == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CTestRect::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CTestRect* CTestRect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestRect* pInstance = new CTestRect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CTestRect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestRect::Clone(void* pArg)
{
    CTestRect* pInstance = new CTestRect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestRect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestRect::Free()
{
    __super::Free();
}
