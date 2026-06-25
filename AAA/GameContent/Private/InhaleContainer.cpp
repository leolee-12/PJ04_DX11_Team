#include "InhaleContainer.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Vacuum.h"
#include "InhaleEffect.h"
#include "TornadoSpinReverse.h"

CInhaleContainer::CInhaleContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CInhaleContainer::CInhaleContainer(const CInhaleContainer& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CInhaleContainer::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CInhaleContainer::Initialize(void* pArg)
{
    INHALE_CONTAINER_DESC* pDesc = static_cast<INHALE_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CInhaleContainer::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CInhaleContainer::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CInhaleContainer::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CInhaleContainer::Render()
{
    __super::Render();

    return S_OK;
}

void CInhaleContainer::On_SuperInhale()
{
    auto iter = m_EffestParts.find(CInhaleEffect::PROTOTYPE_TAG);
    if (iter == m_EffestParts.end())
        return;

    static_cast<CInhaleEffect*>(iter->second)->Set_ColorChange(true);
}

void CInhaleContainer::Off_SuperInhale()
{
    auto iter = m_EffestParts.find(CInhaleEffect::PROTOTYPE_TAG);
    if (iter == m_EffestParts.end())
        return;

    CInhaleEffect* pInhaleEffect = static_cast<CInhaleEffect*>(iter->second);
    pInhaleEffect->Set_ColorChange(false);
    pInhaleEffect->Set_Color(_float3{ 1.f, 1.f, 1.f });
}

HRESULT CInhaleContainer::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CInhaleEffect::PROTOTYPE_TAG, CInhaleEffect::PROTOTYPE_TAG);
    Add_Effect_PartObject(m_iPrototypeLevel, CVacuum::PROTOTYPE_TAG, CVacuum::PROTOTYPE_TAG);
    //Add_Effect_PartObject(m_iPrototypeLevel, CTornadoSpinReverse::PROTOTYPE_TAG, TEXT("Proto_TornadoSpinReverse"));

    return S_OK;
}

CInhaleContainer* CInhaleContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CInhaleContainer* pInstance = new CInhaleContainer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CInhaleContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CInhaleContainer::Clone(void* pArg)
{
    CInhaleContainer* pInstance = new CInhaleContainer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CInhaleContainer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CInhaleContainer::Free()
{
    __super::Free();
}