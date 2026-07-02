#include "SpitObject.h"

#include "GameInstance.h"
#include "GameContent_const.h"
#include "StarParticle.h"
#include "Bubble.h"
#include "StarEmitter.h"

CSpitObject::CSpitObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSpitObject::CSpitObject(const CSpitObject& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSpitObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpitObject::Initialize(void* pArg)
{
    SPIT_OBJECT_DESC* pDesc = static_cast<SPIT_OBJECT_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSpitObject::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSpitObject::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSpitObject::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSpitObject::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSpitObject::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CBubble::PROTOTYPE_TAG, TEXT("Bubble"))))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CStarEmitter::PROTOTYPE_TAG, TEXT("StarEmitter"))))
        return E_FAIL;

    return S_OK;
}

CSpitObject* CSpitObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSpitObject* pInstance = new CSpitObject(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSpitObject");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSpitObject::Clone(void* pArg)
{
    CSpitObject* pInstance = new CSpitObject(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSpitObject");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSpitObject::Free()
{
    __super::Free();
}