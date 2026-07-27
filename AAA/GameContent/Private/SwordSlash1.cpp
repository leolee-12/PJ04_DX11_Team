#include "SwordSlash1.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Common_Ring03.h"

CSwordSlash1::CSwordSlash1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSwordSlash1::CSwordSlash1(const CSwordSlash1& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSwordSlash1::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSwordSlash1::Initialize(void* pArg)
{
    SWORD_SLASH1_DESC* pDesc = static_cast<SWORD_SLASH1_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSwordSlash1::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSwordSlash1::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSwordSlash1::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSwordSlash1::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSwordSlash1::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CCommon_Ring03::PROTOTYPE_TAG, TEXT("Proto_Common_Ring03"));

    return S_OK;
}

CSwordSlash1* CSwordSlash1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSwordSlash1* pInstance = new CSwordSlash1(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSwordSlash1");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSwordSlash1::Clone(void* pArg)
{
    CSwordSlash1* pInstance = new CSwordSlash1(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSwordSlash1");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSwordSlash1::Free()
{
    __super::Free();
}
