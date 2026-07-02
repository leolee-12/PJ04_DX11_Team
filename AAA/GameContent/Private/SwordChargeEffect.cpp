#include "SwordChargeEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SwordCharge.h"

CSwordChargeEffect::CSwordChargeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSwordChargeEffect::CSwordChargeEffect(const CSwordChargeEffect& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSwordChargeEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSwordChargeEffect::Initialize(void* pArg)
{
    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSwordChargeEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSwordChargeEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSwordChargeEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSwordChargeEffect::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSwordChargeEffect::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSwordCharge::PROTOTYPE_TAG, L"SwordCharge1")))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSwordCharge::PROTOTYPE_TAG, L"SwordCharge2")))
        return E_FAIL;

    return S_OK;
}

CSwordChargeEffect* CSwordChargeEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSwordChargeEffect* pInstance = new CSwordChargeEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSwordChargeEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSwordChargeEffect::Clone(void* pArg)
{
    CSwordChargeEffect* pInstance = new CSwordChargeEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSwordChargeEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSwordChargeEffect::Free()
{
    __super::Free();
}
