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

    tDesc.m_iVIBufferLevel = Texture_LoadingUI.iLevelID;
    tDesc.wstrTextureTag = Texture_LoadingUI.szProtoTag;

    tDesc.m_iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.fRotationPerSec = 1.f;
    tDesc.fSpeedPerSec = 1.f;

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
    __super::Late_Update(fTimeDelta);
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
