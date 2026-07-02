#include "Gorilla_Ring.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Shockwave.h"

CGorilla_Ring::CGorilla_Ring(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CGorilla_Ring::CGorilla_Ring(const CGorilla_Ring& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CGorilla_Ring::Initialize_Prototype() { return S_OK; }

HRESULT CGorilla_Ring::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CGorilla_Ring::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CGorilla_Ring::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CGorilla_Ring::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CGorilla_Ring::Render() { __super::Render(); return S_OK; }

HRESULT CGorilla_Ring::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CShockwave::PROTOTYPE_TAG, TEXT("Shockwave"))))
        return E_FAIL;

    return S_OK;
}

CGorilla_Ring* CGorilla_Ring::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGorilla_Ring* pInstance = new CGorilla_Ring(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGorilla_Ring");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CGorilla_Ring::Clone(void* pArg)
{
    CGorilla_Ring* pInstance = new CGorilla_Ring(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CGorilla_Ring");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CGorilla_Ring::Free() { __super::Free(); }