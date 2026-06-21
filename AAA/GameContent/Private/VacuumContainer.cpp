#include "VacuumContainer.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Vacuum.h"
#include "InhaleEffect.h"
#include "TornadoSpinReverse.h"

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

void CVacuumContainer::On_SuperInhale()
{
    auto iter = m_EffestParts.find(CInhaleEffect::PROTOTYPE_TAG);
    if (iter == m_EffestParts.end())
        return;

    static_cast<CInhaleEffect*>(iter->second)->Set_ColorChange(true);
}

void CVacuumContainer::Off_SuperInhale()
{
    auto iter = m_EffestParts.find(CInhaleEffect::PROTOTYPE_TAG);
    if (iter == m_EffestParts.end())
        return;

    CInhaleEffect* pInhaleEffect = static_cast<CInhaleEffect*>(iter->second);
    pInhaleEffect->Set_ColorChange(false);
    pInhaleEffect->Set_Color(_float3{ 1.f, 1.f, 1.f });
}

HRESULT CVacuumContainer::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CInhaleEffect::PROTOTYPE_TAG, CInhaleEffect::PROTOTYPE_TAG);
    Add_Effect_PartObject(m_iPrototypeLevel, CVacuum::PROTOTYPE_TAG, CVacuum::PROTOTYPE_TAG);
    //Add_Effect_PartObject(m_iPrototypeLevel, CTornadoSpinReverse::PROTOTYPE_TAG, TEXT("Proto_TornadoSpinReverse"));

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
        MSG_BOX("Failed to Cloned: CVacuumContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CVacuumContainer::Free()
{
    __super::Free();
}