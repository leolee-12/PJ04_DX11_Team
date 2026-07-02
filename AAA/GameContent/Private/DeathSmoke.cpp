#include "DeathSmoke.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeParticle.h"

CDeathSmoke::CDeathSmoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CDeathSmoke::CDeathSmoke(const CDeathSmoke& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CDeathSmoke::Initialize_Prototype() { return S_OK; }

HRESULT CDeathSmoke::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CDeathSmoke::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CDeathSmoke::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CDeathSmoke::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CDeathSmoke::Render() { __super::Render(); return S_OK; }

HRESULT CDeathSmoke::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound_M"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound_S"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound_UP_S"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound_UP_M"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeCenter"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeCenter_S"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeCenter_M"))))
        return E_FAIL;

    return S_OK;
}

CDeathSmoke* CDeathSmoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDeathSmoke* pInstance = new CDeathSmoke(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CDeathSmoke");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CDeathSmoke::Clone(void* pArg)
{
    CDeathSmoke* pInstance = new CDeathSmoke(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CDeathSmoke");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDeathSmoke::Free() { __super::Free(); }