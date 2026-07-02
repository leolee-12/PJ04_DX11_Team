#include "HitMark.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CHitMark::CHitMark(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Quad{ pDevice, pContext }
{
}
CHitMark::CHitMark(const CHitMark& Prototype)
    : CEffect_Quad(Prototype) 
{
}

HRESULT CHitMark::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CHitMark::Initialize(void* pArg)
{
    HITMARK_QUAD_DESC tDesc{};

    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;

    tDesc.bUseTextureCom = true;                          
    tDesc.iTextureLevel = Texture_CommonHit01.iLevelID;
    tDesc.wstrTextureTag = Texture_CommonHit01.szProtoTag;

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

void CHitMark::Priority_Update(_float fTimeDelta) {
    __super::Priority_Update(fTimeDelta);
}
void CHitMark::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }

void CHitMark::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CHitMark::Render() { __super::Render(); return S_OK; }

CHitMark* CHitMark::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CHitMark* pInstance = new CHitMark(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CHitMark");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CHitMark::Clone(void* pArg)
{
    CHitMark* pInstance = new CHitMark(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CHitMark");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CHitMark::Free() 
{ 
    __super::Free(); 
}