#include "SmokeTail.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CSmokeTail::CSmokeTail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CSmokeTail::CSmokeTail(const CSmokeTail& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CSmokeTail::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSmokeTail::Initialize(void* pArg)
{
    SMOKE_TAIL_DESC tDesc{};

    tDesc.iModelLevel = ETOUI(LEVEL::GAMEPLAY);
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeTail");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = Texture_Common_Flash02.iLevelID;
    tDesc.wstrTextureTag = Texture_Common_Flash02.szProtoTag;

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = Texture_TestMask.iLevelID;
    tDesc.wstrMaskTag = Texture_TestMask.szProtoTag;

    tDesc.bCustomShader = false;
    //tDesc.iShaderLevel = 0;
    //tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CSmokeTail::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSmokeTail::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSmokeTail::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CSmokeTail::Render()
{
    __super::Render();

    return S_OK;
}

CSmokeTail* CSmokeTail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSmokeTail* pInstance = new CSmokeTail(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSmokeTail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSmokeTail::Clone(void* pArg)
{
    CSmokeTail* pInstance = new CSmokeTail(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSmokeTail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSmokeTail::Free()
{
    __super::Free();
}