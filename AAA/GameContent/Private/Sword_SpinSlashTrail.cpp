#include "Sword_SpinSlashTrail.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "Common_SpinSlashTrail.h"

CSword_SpinSlashTrail::CSword_SpinSlashTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CSword_SpinSlashTrail::CSword_SpinSlashTrail(const CSword_SpinSlashTrail& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CSword_SpinSlashTrail::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSword_SpinSlashTrail::Initialize(void* pArg)
{
    SWORD_SLASH1_DESC* pDesc = static_cast<SWORD_SLASH1_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CSword_SpinSlashTrail::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSword_SpinSlashTrail::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bFadeOutRequested == false)
        return;

    _bool bAllFadeOutFinished = true;

    for (auto& [strTag, pPart] : m_EffestParts)
    {
        CCommon_SpinSlashTrail* pSpinSlashTrailPart = dynamic_cast<CCommon_SpinSlashTrail*>(pPart);
        if (pSpinSlashTrailPart == nullptr)
            continue;

        if (Is_EffectPartPlay(strTag) == false)
            continue;

        if (pSpinSlashTrailPart->Is_FadingOut() == false ||
            pSpinSlashTrailPart->Is_FadeOutFinished() == false)
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

void CSword_SpinSlashTrail::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CSword_SpinSlashTrail::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CSword_SpinSlashTrail::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(m_iPrototypeLevel, CCommon_SpinSlashTrail::PROTOTYPE_TAG, CCommon_SpinSlashTrail::PROTOTYPE_TAG);

    return S_OK;
}

void CSword_SpinSlashTrail::Start_FadeOut(_float fFadeOutDuration)
{
    _bool bFadeOutStarted = false;

    for (auto& [strTag, pPart] : m_EffestParts)
    {
        CCommon_SpinSlashTrail* pSpinSlashTrailPart = dynamic_cast<CCommon_SpinSlashTrail*>(pPart);
        if (pSpinSlashTrailPart == nullptr)
            continue;

        if (Is_EffectPartPlay(strTag) == false)
            continue;

        pSpinSlashTrailPart->Start_FadeOut(fFadeOutDuration);
        bFadeOutStarted = true;
    }

    m_bFadeOutRequested = bFadeOutStarted;

    if (bFadeOutStarted == false)
        EffectContainer_Stop();
}

CSword_SpinSlashTrail* CSword_SpinSlashTrail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSword_SpinSlashTrail* pInstance = new CSword_SpinSlashTrail(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSword_SpinSlashTrail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSword_SpinSlashTrail::Clone(void* pArg)
{
    CSword_SpinSlashTrail* pInstance = new CSword_SpinSlashTrail(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSword_SpinSlashTrail");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSword_SpinSlashTrail::Free()
{
    __super::Free();
}
