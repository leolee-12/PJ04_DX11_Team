#include "Vacuum.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CVacuum::CVacuum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CVacuum::CVacuum(const CVacuum& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CVacuum::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CVacuum::Initialize(void* pArg)
{
    VACUUM_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Common_00_Vacuum");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = Texture_Twincle.iLevelID;
    tDesc.wstrTextureTag = Texture_Twincle.szProtoTag;

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = Texture_Twincle.iLevelID;
    tDesc.wstrMaskTag = Texture_Twincle.szProtoTag;

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CVacuum::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CVacuum::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CVacuum::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CVacuum::Render()
{
    __super::Render();

    return S_OK;
}

CVacuum* CVacuum::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVacuum* pInstance = new CVacuum(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CVacuum");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CVacuum::Clone(void* pArg)
{
    CVacuum* pInstance = new CVacuum(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CVacuum");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CVacuum::Free()
{
    __super::Free();
}