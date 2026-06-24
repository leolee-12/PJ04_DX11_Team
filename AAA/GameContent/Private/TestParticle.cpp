#include "TestParticle.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CTestParticle::CTestParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_RectParticle{ pDevice, pContext }
{
}

CTestParticle::CTestParticle(const CTestParticle& Prototype)
    : CEffect_RectParticle(Prototype)
{
}

HRESULT CTestParticle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestParticle::Initialize(void* pArg)
{
    TEST_PARTICLE_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

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

void CTestParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CTestParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CTestParticle::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CTestParticle::Render()
{
    __super::Render();

    return S_OK;
}

CTestParticle* CTestParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestParticle* pInstance = new CTestParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CTestParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestParticle::Clone(void* pArg)
{
    CTestParticle* pInstance = new CTestParticle(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestParticle::Free()
{
    __super::Free();
}
