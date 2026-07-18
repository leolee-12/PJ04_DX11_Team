#include "Armadillo_SpinWind.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Swing_Smoke.h"
#include "SpinWind.h"

CArmadillo_SpinWind::CArmadillo_SpinWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CArmadillo_SpinWind::CArmadillo_SpinWind(const CArmadillo_SpinWind& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CArmadillo_SpinWind::Initialize_Prototype() { return S_OK; }

HRESULT CArmadillo_SpinWind::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CArmadillo_SpinWind::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CArmadillo_SpinWind::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CArmadillo_SpinWind::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CArmadillo_SpinWind::Render() { __super::Render(); return S_OK; }

HRESULT CArmadillo_SpinWind::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSpinWind::PROTOTYPE_TAG, TEXT("SpinWindL"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSpinWind::PROTOTYPE_TAG, TEXT("SpinWindS"))))
        return E_FAIL;

    return S_OK;
}

CArmadillo_SpinWind* CArmadillo_SpinWind::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CArmadillo_SpinWind* pInstance = new CArmadillo_SpinWind(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CArmadillo_SpinWind");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CArmadillo_SpinWind::Clone(void* pArg)
{
    CArmadillo_SpinWind* pInstance = new CArmadillo_SpinWind(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CArmadillo_SpinWind");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CArmadillo_SpinWind::Free() { __super::Free(); }