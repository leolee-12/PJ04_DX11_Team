#include "BombFuseEffect.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeEmitter.h"
#include "Sparkle.h"

CBombFuseEffect::CBombFuseEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CBombFuseEffect::CBombFuseEffect(const CBombFuseEffect& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CBombFuseEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBombFuseEffect::Initialize(void* pArg)
{
    BOMB_FUSE_DESC* pDesc = static_cast<BOMB_FUSE_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CBombFuseEffect::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CBombFuseEffect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CBombFuseEffect::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CBombFuseEffect::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CBombFuseEffect::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeEmitter::PROTOTYPE_TAG, CSmokeEmitter::PROTOTYPE_TAG)))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSparkle::PROTOTYPE_TAG, L"White_Sparkle")))
        return E_FAIL;

    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSparkle::PROTOTYPE_TAG, L"Yellow_Sparkle")))
        return E_FAIL;

    return S_OK;
}

CBombFuseEffect* CBombFuseEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBombFuseEffect* pInstance = new CBombFuseEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBombFuseEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBombFuseEffect::Clone(void* pArg)
{
    CBombFuseEffect* pInstance = new CBombFuseEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBombFuseEffect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBombFuseEffect::Free()
{
    __super::Free();
}