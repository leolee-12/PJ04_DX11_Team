#include "Bubble.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CBubble::CBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Quad{ pDevice, pContext }
{
}
CBubble::CBubble(const CBubble& Prototype)
    : CEffect_Quad(Prototype)
{
}

HRESULT CBubble::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CBubble::Initialize(void* pArg)
{
    BUBBLE_QUAD_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_CommonRing01.iLevelID;
    tDesc.wstrTextureTag = Texture_CommonRing01.szProtoTag;

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

void CBubble::Priority_Update(_float fTimeDelta) {
    __super::Priority_Update(fTimeDelta);
}
void CBubble::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CBubble::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CBubble::Render() { __super::Render(); return S_OK; }

CBubble* CBubble::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBubble* pInstance = new CBubble(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBubble");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBubble::Clone(void* pArg)
{
    CBubble* pInstance = new CBubble(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBubble");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBubble::Free()
{
    __super::Free();
}