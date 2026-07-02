#include "CommonHit.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "StarParticle.h"
#include "HitMark.h"


CCommonHit::CCommonHit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CCommonHit::CCommonHit(const CCommonHit& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CCommonHit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCommonHit::Initialize(void* pArg)
{
    COMMON_HIT_DESC* pDesc = static_cast<COMMON_HIT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CCommonHit::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCommonHit::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCommonHit::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CCommonHit::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CCommonHit::Ready_EffectPartObjects()
{ 

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarParticle::PROTOTYPE_TAG, TEXT("Proto_StarSmooth"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CHitMark::PROTOTYPE_TAG, TEXT("HitMark"))))
        return E_FAIL;

    return S_OK;
}

CCommonHit* CCommonHit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCommonHit* pInstance = new CCommonHit(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCommonHit");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCommonHit::Clone(void* pArg)
{
    CCommonHit* pInstance = new CCommonHit(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCommonHit");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCommonHit::Free()
{
    __super::Free();
}