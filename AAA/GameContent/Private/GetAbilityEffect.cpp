#include "GetAbilityEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Star2DParticle.h"

CGetAbilityEffect::CGetAbilityEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CGetAbilityEffect::CGetAbilityEffect(const CGetAbilityEffect& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CGetAbilityEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CGetAbilityEffect::Initialize(void* pArg)
{
    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CGetAbilityEffect::Priority_Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);
    __super::Priority_Update(fTimeDelta);
}

void CGetAbilityEffect::Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);
    __super::Update(fTimeDelta);
}

void CGetAbilityEffect::Late_Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);
    __super::Late_Update(fTimeDelta);
}

HRESULT CGetAbilityEffect::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CGetAbilityEffect::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"BigStar1")))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"BigStar2")))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"BigStar3")))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"BigStar4")))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"BigStar5")))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"SmallStar1")))
        return E_FAIL;                                                                    
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"SmallStar2")))
        return E_FAIL;                                                                    
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"SmallStar3")))
        return E_FAIL;                                                                    
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"SmallStar4")))
        return E_FAIL;                                                                    
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStar2DParticle::PROTOTYPE_TAG, L"SmallStar5")))
        return E_FAIL;

    return S_OK;
}

_float CGetAbilityEffect::Resolve_TimeDelta(_float fTimeDelta)
{
    return m_pGameInstance_Proxy->Get_RawTimeDelta(TEXT("Timer_60"));
}

CGetAbilityEffect* CGetAbilityEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGetAbilityEffect* pInstance = new CGetAbilityEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CGetAbilityEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CGetAbilityEffect::Clone(void* pArg)
{
    CGetAbilityEffect* pInstance = new CGetAbilityEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CGetAbilityEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGetAbilityEffect::Free()
{
    __super::Free();
}
