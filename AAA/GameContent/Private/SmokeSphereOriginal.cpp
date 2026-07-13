#include "SmokeSphereOriginal.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CSmokeSphereOriginal::CSmokeSphereOriginal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CSmokeSphereOriginal::CSmokeSphereOriginal(const CSmokeSphereOriginal& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CSmokeSphereOriginal::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSmokeSphereOriginal::Initialize(void* pArg)
{
    SMOKE_SPHERE_ORIGIANL_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeSphereOriginal");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    //tDesc.iShaderLevel = 0;
    //tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;   

    return S_OK;
}

void CSmokeSphereOriginal::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSmokeSphereOriginal::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSmokeSphereOriginal::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CSmokeSphereOriginal::Render()
{
    __super::Render();

    return S_OK;
}

CSmokeSphereOriginal* CSmokeSphereOriginal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSmokeSphereOriginal* pInstance = new CSmokeSphereOriginal(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSmokeSphereOriginal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSmokeSphereOriginal::Clone(void* pArg)
{
    CSmokeSphereOriginal* pInstance = new CSmokeSphereOriginal(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSmokeSphereOriginal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSmokeSphereOriginal::Free()
{
    __super::Free();
}