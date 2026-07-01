#include "SmokeParticle.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CSmokeParticle::CSmokeParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CSmokeParticle::CSmokeParticle(const CSmokeParticle& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CSmokeParticle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSmokeParticle::Initialize(void* pArg)
{
    SMOKE_PARTICLE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeSphereOriginal");

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

void CSmokeParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSmokeParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSmokeParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CSmokeParticle::Render()
{
    __super::Render();

    return S_OK;
}

CSmokeParticle* CSmokeParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSmokeParticle* pInstance = new CSmokeParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSmokeParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSmokeParticle::Clone(void* pArg)
{
    CSmokeParticle* pInstance = new CSmokeParticle(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSmokeParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSmokeParticle::Free()
{
    __super::Free();
}