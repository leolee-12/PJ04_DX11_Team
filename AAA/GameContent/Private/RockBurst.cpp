#include "RockBurst.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "RockFloorEffect.h"

CRockBurst::CRockBurst(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext) {
}

CRockBurst::CRockBurst(const CRockBurst& Prototype)
    : CEffect_Container(Prototype) {
}

HRESULT CRockBurst::Initialize_Prototype() { return S_OK; }

HRESULT CRockBurst::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CRockBurst::Priority_Update(_float fTimeDelta) { __super::Priority_Update(fTimeDelta); }
void CRockBurst::Update(_float fTimeDelta) { __super::Update(fTimeDelta); }
void CRockBurst::Late_Update(_float fTimeDelta) { __super::Late_Update(fTimeDelta); }
HRESULT CRockBurst::Render() { __super::Render(); return S_OK; }

HRESULT CRockBurst::Ready_EffectPartObjects()
{
    // partTag "RockFloor" = json EffectPartObjects 키와 매칭(지금은 json 비워서 코드 기본값 사용)
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRockFloorEffect::PROTOTYPE_TAG, TEXT("RockFloor"))))
        return E_FAIL;

    return S_OK;
}

CRockBurst* CRockBurst::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRockBurst* pInstance = new CRockBurst(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CRockBurst");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CRockBurst::Clone(void* pArg)
{
    CRockBurst* pInstance = new CRockBurst(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CRockBurst");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRockBurst::Free() { __super::Free(); }