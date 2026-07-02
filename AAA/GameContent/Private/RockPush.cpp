#include "RockPush.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeParticle.h"

CRockPush::CRockPush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CRockPush::CRockPush(const CRockPush& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CRockPush::Initialize_Prototype() { return S_OK; }

HRESULT CRockPush::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CRockPush::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CRockPush::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CRockPush::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CRockPush::Render() { __super::Render(); return S_OK; }

HRESULT CRockPush::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("SmokeRound"))))
        return E_FAIL;

    return S_OK;
}

CRockPush* CRockPush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRockPush* pInstance = new CRockPush(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRockPush");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRockPush::Clone(void* pArg)
{
    CRockPush* pInstance = new CRockPush(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRockPush");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRockPush::Free() { __super::Free(); }