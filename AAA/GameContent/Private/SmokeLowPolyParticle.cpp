#include "StarParticle.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CStarParticle::CStarParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CStarParticle::CStarParticle(const CStarParticle& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CStarParticle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CStarParticle::Initialize(void* pArg)
{
    STAR_PARTICLE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_StarSmooth");

    if (pArg != nullptr)
    {
        auto pIn = static_cast<STAR_PARTICLE_DESC*>(pArg);
        if (!pIn->wstrModelTag.empty())
            tDesc.wstrModelTag = pIn->wstrModelTag;
    }

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

void CStarParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CStarParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CStarParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CStarParticle::Render()
{
    __super::Render();

    return S_OK;
}

CStarParticle* CStarParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStarParticle* pInstance = new CStarParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CStarParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CStarParticle::Clone(void* pArg)
{
    CStarParticle* pInstance = new CStarParticle(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CStarParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStarParticle::Free()
{
    __super::Free();
}