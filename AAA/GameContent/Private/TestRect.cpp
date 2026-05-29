#include "TestRect.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "Navigation.h"

CTestRect::CTestRect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{

}

CTestRect::CTestRect(const CTestRect& Prototype)
    : CGameObject(Prototype)
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

    if (FAILED(m_pShaderCom->Begin(0)))
        return E_FAIL;
   
    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
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
