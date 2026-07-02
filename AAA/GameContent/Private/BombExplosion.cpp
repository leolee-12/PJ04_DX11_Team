#include "BombExplosion.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "SmokeSphereOriginal.h"
#include "SmokeParticle.h"
#include "SphereParticle.h"
#include "Common_SphereNoise.h"

CBombExplosion::CBombExplosion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CBombExplosion::CBombExplosion(const CBombExplosion& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CBombExplosion::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBombExplosion::Initialize(void* pArg)
{
    DESPAWN_EFFECT_DESC* pDesc = static_cast<DESPAWN_EFFECT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CBombExplosion::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CBombExplosion::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CBombExplosion::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CBombExplosion::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CBombExplosion::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeParticle::PROTOTYPE_TAG, TEXT("Proto_SmokeParticle"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSphereParticle::PROTOTYPE_TAG, TEXT("Proto_SphereParticle"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CCommon_SphereNoise::PROTOTYPE_TAG, TEXT("Proto_SphereNoise"))))
        return E_FAIL;

    return S_OK;
}

CBombExplosion* CBombExplosion::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBombExplosion* pInstance = new CBombExplosion(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBombExplosion");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBombExplosion::Clone(void* pArg)
{
    CBombExplosion* pInstance = new CBombExplosion(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBombExplosion");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBombExplosion::Free()
{
    __super::Free();
}