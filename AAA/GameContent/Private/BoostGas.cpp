#include "BoostGas.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeEmitter.h"

CBoostGas::CBoostGas(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CBoostGas::CBoostGas(const CBoostGas& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CBoostGas::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoostGas::Initialize(void* pArg)
{
    BOOST_GAS_DESC* pDesc = static_cast<BOOST_GAS_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CBoostGas::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CBoostGas::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CBoostGas::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CBoostGas::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CBoostGas::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeEmitter::PROTOTYPE_TAG, CSmokeEmitter::PROTOTYPE_TAG)))
        return E_FAIL;
 
    return S_OK;
}

CBoostGas* CBoostGas::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoostGas* pInstance = new CBoostGas(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBoostGas");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBoostGas::Clone(void* pArg)
{
    CBoostGas* pInstance = new CBoostGas(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBoostGas");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBoostGas::Free()
{
    __super::Free();
}