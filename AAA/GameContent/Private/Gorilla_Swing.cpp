#include "Gorilla_Swing.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Swing_Smoke.h"
#include "SpinWind.h"

CGorilla_Swing::CGorilla_Swing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CGorilla_Swing::CGorilla_Swing(const CGorilla_Swing& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CGorilla_Swing::Initialize_Prototype() { return S_OK; }

HRESULT CGorilla_Swing::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CGorilla_Swing::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CGorilla_Swing::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CGorilla_Swing::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CGorilla_Swing::Render() { __super::Render(); return S_OK; }

HRESULT CGorilla_Swing::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSwing_Smoke::PROTOTYPE_TAG, TEXT("SwingSmoke_L"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSwing_Smoke::PROTOTYPE_TAG, TEXT("SwingSmoke_M"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSwing_Smoke::PROTOTYPE_TAG, TEXT("SwingSmoke_S"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSpinWind::PROTOTYPE_TAG, TEXT("SwingWind"))))
        return E_FAIL;

    return S_OK;
}

CGorilla_Swing* CGorilla_Swing::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGorilla_Swing* pInstance = new CGorilla_Swing(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGorilla_Swing");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CGorilla_Swing::Clone(void* pArg)
{
    CGorilla_Swing* pInstance = new CGorilla_Swing(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CGorilla_Swing");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGorilla_Swing::Free() { __super::Free(); }