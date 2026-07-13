#include "Common_SphereNoise.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCommon_SphereNoise::CCommon_SphereNoise(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CCommon_SphereNoise::CCommon_SphereNoise(const CCommon_SphereNoise& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CCommon_SphereNoise::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCommon_SphereNoise::Initialize(void* pArg)
{
    SPHERE_NOISE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SphereNoise");

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

void CCommon_SphereNoise::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommon_SphereNoise::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommon_SphereNoise::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCommon_SphereNoise::Render()
{
    __super::Render();

    return S_OK;
}

CCommon_SphereNoise* CCommon_SphereNoise::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommon_SphereNoise* pInstance = new CCommon_SphereNoise(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommon_SphereNoise");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommon_SphereNoise::Clone(void* pArg)
{
    CCommon_SphereNoise* pInstance = new CCommon_SphereNoise(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommon_SphereNoise");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommon_SphereNoise::Free()
{
    __super::Free();
}