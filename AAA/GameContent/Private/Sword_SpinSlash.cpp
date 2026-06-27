#include "Sword_SpinSlash.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Common_SpinSlash.h"

CSword_SpinSlash::CSword_SpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSword_SpinSlash::CSword_SpinSlash(const CSword_SpinSlash& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSword_SpinSlash::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSword_SpinSlash::Initialize(void* pArg)
{
    SWORD_SPINSLASH_DESC* pDesc = static_cast<SWORD_SPINSLASH_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSword_SpinSlash::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSword_SpinSlash::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bFadeOutRequested == false)
        return;

    _bool bAllFadeOutFinished = true;

    for (auto& [strTag, pPart] : m_EffestParts)
    {
        CCommon_SpinSlash* pSpinSlashPart = dynamic_cast<CCommon_SpinSlash*>(pPart);
        if (pSpinSlashPart == nullptr)
            continue;

        if (Is_EffectPartPlay(strTag) == false)
            continue;

        if (pSpinSlashPart->Is_FadingOut() == false)
        {
            bAllFadeOutFinished = false;
            break;
        }

        if (pSpinSlashPart->Is_FadeOutFinished() == false)
        {
            bAllFadeOutFinished = false;
            break;
        }
    }

    if (bAllFadeOutFinished == true)
    {
        m_bFadeOutRequested = false;
        EffectContainer_Stop();
    }
}

void CSword_SpinSlash::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSword_SpinSlash::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSword_SpinSlash::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CCommon_SpinSlash::PROTOTYPE_TAG, L"Proto_Common_SpinSlash_1");
    Add_Effect_PartObject(m_iPrototypeLevel, CCommon_SpinSlash::PROTOTYPE_TAG, L"Proto_Common_SpinSlash_2");

    return S_OK;
}

void CSword_SpinSlash::Start_FadeOut(_float fFadeOutDuration)
{
    _bool bStarted = false;

    for (auto& [strTag, pPart] : m_EffestParts)
    {
        CCommon_SpinSlash* pSpinSlashPart = dynamic_cast<CCommon_SpinSlash*>(pPart);
        if (pSpinSlashPart != nullptr && Is_EffectPartPlay(strTag) == true)
        {
            pSpinSlashPart->Start_FadeOut(fFadeOutDuration);
            bStarted = true;
        }
    }

    m_bFadeOutRequested = bStarted;

    if (bStarted == false)
        EffectContainer_Stop();
}

CSword_SpinSlash* CSword_SpinSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSword_SpinSlash* pInstance = new CSword_SpinSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSword_SpinSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSword_SpinSlash::Clone(void* pArg)
{
    CSword_SpinSlash* pInstance = new CSword_SpinSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSword_SpinSlash");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSword_SpinSlash::Free()
{
    __super::Free();
}
