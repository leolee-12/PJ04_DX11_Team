#include "AbilityBubbleEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "StarParticle.h"
#include "Bubble.h"
#include "StarEmitter.h"

CAbilityBubbleEffect::CAbilityBubbleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CAbilityBubbleEffect::CAbilityBubbleEffect(const CAbilityBubbleEffect& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CAbilityBubbleEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAbilityBubbleEffect::Initialize(void* pArg)
{
    ABILITYBUBBLE_EFFECT_DESC* pDesc = static_cast<ABILITYBUBBLE_EFFECT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CAbilityBubbleEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CAbilityBubbleEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CAbilityBubbleEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CAbilityBubbleEffect::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CAbilityBubbleEffect::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CBubble::PROTOTYPE_TAG, TEXT("Bubble"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarEmitter::PROTOTYPE_TAG, TEXT("StarEmitter00"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarEmitter::PROTOTYPE_TAG, TEXT("StarEmitter01"))))
        return E_FAIL;

    return S_OK;
}

CAbilityBubbleEffect* CAbilityBubbleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAbilityBubbleEffect* pInstance = new CAbilityBubbleEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CAbilityBubbleEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAbilityBubbleEffect::Clone(void* pArg)
{
    CAbilityBubbleEffect* pInstance = new CAbilityBubbleEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CAbilityBubbleEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAbilityBubbleEffect::Free()
{
    __super::Free();
}