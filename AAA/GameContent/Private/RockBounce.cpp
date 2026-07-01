#include "RockBounce.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeParticle.h"

CRockBounce::CRockBounce(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CRockBounce::CRockBounce(const CRockBounce& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CRockBounce::Initialize_Prototype() { return S_OK; }

HRESULT CRockBounce::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CRockBounce::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CRockBounce::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CRockBounce::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CRockBounce::Render() { __super::Render(); return S_OK; }

HRESULT CRockBounce::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound"))))
        return E_FAIL;

    return S_OK;
}

CRockBounce* CRockBounce::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRockBounce* pInstance = new CRockBounce(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRockBounce");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRockBounce::Clone(void* pArg)
{
    CRockBounce* pInstance = new CRockBounce(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRockBounce");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRockBounce::Free() { __super::Free(); }