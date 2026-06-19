#include "SmokeLowPoly.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CSmokeLowPoly::CSmokeLowPoly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CSmokeLowPoly::CSmokeLowPoly(const CSmokeLowPoly& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CSmokeLowPoly::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSmokeLowPoly::Initialize(void* pArg)
{
    SMOKE_LOW_POLY_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeLowPoly");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

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

void CSmokeLowPoly::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSmokeLowPoly::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSmokeLowPoly::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CSmokeLowPoly::Render()
{
    __super::Render();

    return S_OK;
}

CSmokeLowPoly* CSmokeLowPoly::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSmokeLowPoly* pInstance = new CSmokeLowPoly(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSmokeLowPoly");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSmokeLowPoly::Clone(void* pArg)
{
    CSmokeLowPoly* pInstance = new CSmokeLowPoly(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSmokeLowPoly");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSmokeLowPoly::Free()
{
    __super::Free();
}