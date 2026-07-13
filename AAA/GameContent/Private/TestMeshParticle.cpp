#include "TestMeshParticle.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CTestMeshParticle::CTestMeshParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshParticle{ pDevice, pContext }
{
}

CTestMeshParticle::CTestMeshParticle(const CTestMeshParticle& Prototype)
    : CEffect_MeshParticle(Prototype)
{
}

HRESULT CTestMeshParticle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestMeshParticle::Initialize(void* pArg)
{
    TEST_MESHPARTICLE_DESC tDesc{};

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

void CTestMeshParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CTestMeshParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CTestMeshParticle::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestMeshParticle::Render()
{
    __super::Render();

    return S_OK;
}

CTestMeshParticle* CTestMeshParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestMeshParticle* pInstance = new CTestMeshParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CTestMeshParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestMeshParticle::Clone(void* pArg)
{
    CTestMeshParticle* pInstance = new CTestMeshParticle(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestMeshParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestMeshParticle::Free()
{
    __super::Free();
}
