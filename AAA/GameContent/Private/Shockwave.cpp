#include "Shockwave.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CShockwave::CShockwave(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CShockwave::CShockwave(const CShockwave& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CShockwave::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CShockwave::Initialize(void* pArg)
{
    SHOCKWAVE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = MODEL_PROTO_TAG;

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CShockwave::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CShockwave::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CShockwave::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CShockwave::Render()
{
    __super::Render();

    return S_OK;
}

CShockwave* CShockwave::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CShockwave* pInstance = new CShockwave(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CShockwave");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CShockwave::Clone(void* pArg)
{
    CShockwave* pInstance = new CShockwave(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CShockwave");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CShockwave::Free()
{
    __super::Free();
}