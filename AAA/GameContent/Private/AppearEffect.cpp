#include "AppearEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CAppearEffect::CAppearEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CAppearEffect::CAppearEffect(const CAppearEffect& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CAppearEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAppearEffect::Initialize(void* pArg)
{
    APPEAR_EFFECT_DESC* pDesc = static_cast<APPEAR_EFFECT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CAppearEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CAppearEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CAppearEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CAppearEffect::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CAppearEffect::Ready_EffectPartObjects()
{


    return S_OK;
}

CAppearEffect* CAppearEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAppearEffect* pInstance = new CAppearEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CAppearEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAppearEffect::Clone(void* pArg)
{
    CAppearEffect* pInstance = new CAppearEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CAppearEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAppearEffect::Free()
{
    __super::Free();
}