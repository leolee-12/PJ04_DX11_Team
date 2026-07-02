#include "RockPull.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeParticle.h"

CRockPull::CRockPull(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CRockPull::CRockPull(const CRockPull& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CRockPull::Initialize_Prototype() { return S_OK; }

HRESULT CRockPull::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CRockPull::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CRockPull::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CRockPull::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CRockPull::Render() { __super::Render(); return S_OK; }

HRESULT CRockPull::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound"))))
        return E_FAIL;

    return S_OK;
}

CRockPull* CRockPull::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRockPull* pInstance = new CRockPull(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRockPull");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRockPull::Clone(void* pArg)
{
    CRockPull* pInstance = new CRockPull(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRockPull");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRockPull::Free() { __super::Free(); }