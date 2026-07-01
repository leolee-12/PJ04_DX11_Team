#include "DespawnEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "SmokeParticle.h"
#include "StarParticle.h"

CDespawnEffect::CDespawnEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CDespawnEffect::CDespawnEffect(const CDespawnEffect& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CDespawnEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDespawnEffect::Initialize(void* pArg)
{
    DESPAWN_EFFECT_DESC* pDesc = static_cast<DESPAWN_EFFECT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CDespawnEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CDespawnEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CDespawnEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CDespawnEffect::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CDespawnEffect::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("Proto_SmokeParticle"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarParticle::PROTOTYPE_TAG, TEXT("Proto_StarParticle"))))
        return E_FAIL;

    return S_OK;
}

CDespawnEffect* CDespawnEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDespawnEffect* pInstance = new CDespawnEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CDespawnEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CDespawnEffect::Clone(void* pArg)
{
    CDespawnEffect* pInstance = new CDespawnEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CDespawnEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CDespawnEffect::Free()
{
    __super::Free();
}