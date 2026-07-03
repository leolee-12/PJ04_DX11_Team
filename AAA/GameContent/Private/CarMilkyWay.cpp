#include "CarMilkyWay.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Car_00_MilkyWay.h"

CCarMilkyWay::CCarMilkyWay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CCarMilkyWay::CCarMilkyWay(const CCarMilkyWay& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CCarMilkyWay::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCarMilkyWay::Initialize(void* pArg)
{
    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CCarMilkyWay::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCarMilkyWay::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCarMilkyWay::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CCarMilkyWay::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CCarMilkyWay::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CCar_00_MilkyWay::PROTOTYPE_TAG, TEXT("BoostWind1"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CCar_00_MilkyWay::PROTOTYPE_TAG, TEXT("BoostWind2"))))
        return E_FAIL;

    return S_OK;
}

CCarMilkyWay* CCarMilkyWay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCarMilkyWay* pInstance = new CCarMilkyWay(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCarMilkyWay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCarMilkyWay::Clone(void* pArg)
{
    CCarMilkyWay* pInstance = new CCarMilkyWay(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCarMilkyWay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCarMilkyWay::Free()
{
    __super::Free();
}