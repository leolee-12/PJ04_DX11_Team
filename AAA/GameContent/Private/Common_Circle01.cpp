#include "Common_Circle01.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_Circle01::CCommon_Circle01(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCommon_Circle01::CCommon_Circle01(const CCommon_Circle01& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCommon_Circle01::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_Circle01::Initialize(void* pArg)
{
    COMMON_CIRCLE01_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_Circle01");

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

void CCommon_Circle01::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_Circle01::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommon_Circle01::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_Circle01::Render()
{
    __super::Render();

    return S_OK;
}

CCommon_Circle01* CCommon_Circle01::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_Circle01* pInstance = new CCommon_Circle01(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_Circle01");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_Circle01::Clone(void* pArg)
{
    CCommon_Circle01* pInstance = new CCommon_Circle01(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_Circle01");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_Circle01::Free()
{
    __super::Free();
}