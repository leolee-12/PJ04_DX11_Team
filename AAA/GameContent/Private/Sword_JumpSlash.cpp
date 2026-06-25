#include "Sword_JumpSlash.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Common_JumpSlash.h"

CSword_JumpSlash::CSword_JumpSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSword_JumpSlash::CSword_JumpSlash(const CSword_JumpSlash& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSword_JumpSlash::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSword_JumpSlash::Initialize(void* pArg)
{
    SWORD_SLASH1_DESC* pDesc = static_cast<SWORD_SLASH1_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSword_JumpSlash::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSword_JumpSlash::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSword_JumpSlash::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSword_JumpSlash::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSword_JumpSlash::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CCommon_JumpSlash::PROTOTYPE_TAG, CCommon_JumpSlash::PROTOTYPE_TAG);

    return S_OK;
}

CSword_JumpSlash* CSword_JumpSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSword_JumpSlash* pInstance = new CSword_JumpSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSword_JumpSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSword_JumpSlash::Clone(void* pArg)
{
    CSword_JumpSlash* pInstance = new CSword_JumpSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSword_JumpSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSword_JumpSlash::Free()
{
    __super::Free();
}