#include "Common_Ring03.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_Ring03::CCommon_Ring03(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCommon_Ring03::CCommon_Ring03(const CCommon_Ring03& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCommon_Ring03::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_Ring03::Initialize(void* pArg)
{
    COMMON_RING03_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_Ring03");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_SwordSlash2.iLevelID;
    tDesc.wstrTextureTag = Texture_SwordSlash2.szProtoTag;

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

void CCommon_Ring03::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_Ring03::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommon_Ring03::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_Ring03::Render()
{
    __super::Render();

    return S_OK;
}

CCommon_Ring03* CCommon_Ring03::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_Ring03* pInstance = new CCommon_Ring03(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_Ring03");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_Ring03::Clone(void* pArg)
{
    CCommon_Ring03* pInstance = new CCommon_Ring03(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_Ring03");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_Ring03::Free()
{
    __super::Free();
}