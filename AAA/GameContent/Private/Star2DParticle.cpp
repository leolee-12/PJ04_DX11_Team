#include "Star2DParticle.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CStar2DParticle::CStar2DParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_RectParticle{ pDevice, pContext }
{
}

CStar2DParticle::CStar2DParticle(const CStar2DParticle& Prototype)
    : CEffect_RectParticle(Prototype)
{
}

HRESULT CStar2DParticle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CStar2DParticle::Initialize(void* pArg)
{
    EFFECT_RECTPARTICLE_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_Star2D.iLevelID;
    tDesc.wstrTextureTag = Texture_Star2D.szProtoTag;

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

void CStar2DParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CStar2DParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CStar2DParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CStar2DParticle::Render()
{
    __super::Render();

    return S_OK;
}

CStar2DParticle* CStar2DParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStar2DParticle* pInstance = new CStar2DParticle(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CStar2DParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CStar2DParticle::Clone(void* pArg)
{
    CStar2DParticle* pInstance = new CStar2DParticle(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CStar2DParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStar2DParticle::Free()
{
    __super::Free();
}