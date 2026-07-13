#include "Common_JumpSlash.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_JumpSlash::CCommon_JumpSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCommon_JumpSlash::CCommon_JumpSlash(const CCommon_JumpSlash& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCommon_JumpSlash::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_JumpSlash::Initialize(void* pArg)
{
    COMMON_JUMPSLASH_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_JumpSlash");

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

void CCommon_JumpSlash::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_JumpSlash::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommon_JumpSlash::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_JumpSlash::Render()
{
    __super::Render();

    return S_OK;
}

CCommon_JumpSlash* CCommon_JumpSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_JumpSlash* pInstance = new CCommon_JumpSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_JumpSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_JumpSlash::Clone(void* pArg)
{
    CCommon_JumpSlash* pInstance = new CCommon_JumpSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_JumpSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_JumpSlash::Free()
{
    __super::Free();
}