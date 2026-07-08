#include "Sparkle.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CSparkle::CSparkle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Quad{ pDevice, pContext }
{
}
CSparkle::CSparkle(const CSparkle& Prototype)
    : CEffect_Quad(Prototype)
{
}

HRESULT CSparkle::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSparkle::Initialize(void* pArg)
{
    SPARKLE_QUAD_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_CommonSparkle02.iLevelID;
    tDesc.wstrTextureTag = Texture_CommonSparkle02.szProtoTag;

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

void CSparkle::Priority_Update(_float fTimeDelta) {
    __super::Priority_Update(fTimeDelta);
}
void CSparkle::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CSparkle::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CSparkle::Render() { __super::Render(); return S_OK; }

CSparkle* CSparkle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSparkle* pInstance = new CSparkle(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSparkle");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CSparkle::Clone(void* pArg)
{
    CSparkle* pInstance = new CSparkle(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSparkle");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CSparkle::Free()
{
    __super::Free();
}