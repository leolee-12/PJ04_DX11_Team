#include "Common_Sphere02.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_Sphere02::CCommon_Sphere02(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CCommon_Sphere02::CCommon_Sphere02(const CCommon_Sphere02& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CCommon_Sphere02::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_Sphere02::Initialize(void* pArg)
{
    COMMON_SPHERE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_CommonSphere");

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

void CCommon_Sphere02::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_Sphere02::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommon_Sphere02::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CCommon_Sphere02::Render()
{
    __super::Render();

    return S_OK;
}

CCommon_Sphere02* CCommon_Sphere02::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_Sphere02* pInstance = new CCommon_Sphere02(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_Sphere02");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_Sphere02::Clone(void* pArg)
{
    CCommon_Sphere02* pInstance = new CCommon_Sphere02(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_Sphere02");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_Sphere02::Free()
{
    __super::Free();
}