#include "CoasterWind.h"

#include "GameContent_const.h"
#include "RectEmitterCommon.h"

CCoasterWind::CCoasterWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CCoasterWind::CCoasterWind(const CCoasterWind& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CCoasterWind::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCoasterWind::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CCoasterWind::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCoasterWind::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCoasterWind::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CCoasterWind::Render()
{
    return __super::Render();
}

HRESULT CCoasterWind::Ready_EffectPartObjects()
{
    CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tDesc{};
    tDesc.iVIBufferLevel = VI_Rect.iLevelID;
    tDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_CoasterWind.iLevelID;
    tDesc.wstrTextureTag = Texture_CoasterWind.szProtoTag;
    tDesc.bUseMaskCom = false;
    tDesc.bCustomShader = false;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, L"WindStrip", &tDesc)))
        return E_FAIL;

    return S_OK;
}

CCoasterWind* CCoasterWind::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCoasterWind* pInstance = new CCoasterWind(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCoasterWind");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCoasterWind::Clone(void* pArg)
{
    CCoasterWind* pInstance = new CCoasterWind(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCoasterWind");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCoasterWind::Free()
{
    __super::Free();
}
