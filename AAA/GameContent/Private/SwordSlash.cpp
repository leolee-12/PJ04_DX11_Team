#include "SwordSlash.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Common_Curve03.h"
#include "Common_Circle01.h"

CSwordSlash::CSwordSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSwordSlash::CSwordSlash(const CSwordSlash& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSwordSlash::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSwordSlash::Initialize(void* pArg)
{
    SWORD_SLASH_DESC* pDesc = static_cast<SWORD_SLASH_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSwordSlash::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSwordSlash::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSwordSlash::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSwordSlash::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSwordSlash::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CCommon_Curve03::PROTOTYPE_TAG, TEXT("Proto_Common_Curve03"));
    //Add_Effect_PartObject(m_iPrototypeLevel, CCommon_Circle01::PROTOTYPE_TAG, TEXT("Proto_Common_Circle01"));

    return S_OK;
}

CSwordSlash* CSwordSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSwordSlash* pInstance = new CSwordSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSwordSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSwordSlash::Clone(void* pArg)
{
    CSwordSlash* pInstance = new CSwordSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSwordSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSwordSlash::Free()
{
    __super::Free();
}