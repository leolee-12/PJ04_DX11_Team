#include "Armadillo_WallImpact.h"
#include "GameContent_const.h"

#include "SwordCharge.h"

CArmadillo_WallImpact::CArmadillo_WallImpact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CArmadillo_WallImpact::CArmadillo_WallImpact(const CArmadillo_WallImpact& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CArmadillo_WallImpact::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CArmadillo_WallImpact::Initialize(void* pArg)
{
    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CArmadillo_WallImpact::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CArmadillo_WallImpact::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CArmadillo_WallImpact::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CArmadillo_WallImpact::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CArmadillo_WallImpact::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSwordCharge::PROTOTYPE_TAG, L"Impact")))
        return E_FAIL;

    return S_OK;
}

CArmadillo_WallImpact* CArmadillo_WallImpact::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CArmadillo_WallImpact* pInstance = new CArmadillo_WallImpact(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CArmadillo_WallImpact");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CArmadillo_WallImpact::Clone(void* pArg)
{
    CArmadillo_WallImpact* pInstance = new CArmadillo_WallImpact(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CArmadillo_WallImpact");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CArmadillo_WallImpact::Free()
{
    __super::Free();
}
