#include "Common_SpinSlashTrail.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_SpinSlashTrail::CCommon_SpinSlashTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCommon_SpinSlashTrail::CCommon_SpinSlashTrail(const CCommon_SpinSlashTrail& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCommon_SpinSlashTrail::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_SpinSlashTrail::Initialize(void* pArg)
{
    COMMON_SPINSSLASHTRAIL_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_SpinSlashTrail");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

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

void CCommon_SpinSlashTrail::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_SpinSlashTrail::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommon_SpinSlashTrail::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_SpinSlashTrail::Render()
{
    __super::Render();

    return S_OK;
}

CCommon_SpinSlashTrail* CCommon_SpinSlashTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_SpinSlashTrail* pInstance = new CCommon_SpinSlashTrail(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_SpinSlashTrail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_SpinSlashTrail::Clone(void* pArg)
{
    CCommon_SpinSlashTrail* pInstance = new CCommon_SpinSlashTrail(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_SpinSlashTrail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_SpinSlashTrail::Free()
{
    __super::Free();
}