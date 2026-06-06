#include "VacuumContainer.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Vacuum.h"

CVacuumContainer::CVacuumContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CVacuumContainer::CVacuumContainer(const CVacuumContainer& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CVacuumContainer::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CVacuumContainer::Initialize(void* pArg)
{
    VACUUM_CONTAINER_DESC* pDesc = static_cast<VACUUM_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CVacuumContainer::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CVacuumContainer::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CVacuumContainer::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CVacuumContainer::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CVacuumContainer::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(ETOUI(LEVEL::GAMEPLAY), CVacuum::PROTOTYPE_TAG, TEXT("Proto_Vacuum"));

    return S_OK;
}

CVacuumContainer* CVacuumContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVacuumContainer* pInstance = new CVacuumContainer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CVacuumContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CVacuumContainer::Clone(void* pArg)
{
    CVacuumContainer* pInstance = new CVacuumContainer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestEffectQuad");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CVacuumContainer::Free()
{
    __super::Free();
}