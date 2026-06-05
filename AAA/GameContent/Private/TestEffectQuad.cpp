#include "TestEffectQuad.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CTestEffectQuad::CTestEffectQuad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Quad{ pDevice, pContext }
{
}

CTestEffectQuad::CTestEffectQuad(const CTestEffectQuad& Prototype)
    : CEffect_Quad(Prototype)
{
}

HRESULT CTestEffectQuad::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestEffectQuad::Initialize(void* pArg)
{
    TEST_EFFECT_QUAD_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_Common_Flash02.iLevelID;
    tDesc.wstrTextureTag = Texture_Common_Flash02.szProtoTag;

    tDesc.bUseMaskCom = true;
    tDesc.iMaskLevel = Texture_TestMask.iLevelID;
    tDesc.wstrMaskTag = Texture_TestMask.szProtoTag;

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CTestEffectQuad::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CTestEffectQuad::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CTestEffectQuad::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CTestEffectQuad::Render()
{
    __super::Render();

    return S_OK;
}

CTestEffectQuad* CTestEffectQuad::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestEffectQuad* pInstance = new CTestEffectQuad(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CTestEffectQuad");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestEffectQuad::Clone(void* pArg)
{
    CTestEffectQuad* pInstance = new CTestEffectQuad(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestEffectQuad");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestEffectQuad::Free()
{
    __super::Free();
}
