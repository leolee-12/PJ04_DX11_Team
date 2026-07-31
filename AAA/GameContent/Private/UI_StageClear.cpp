#include "UI_StageClear.h"
#include "UIPartObject.h"
#include "GameInstance.h"
#include "GameContent_Events.h"


CUI_StageClear::CUI_StageClear(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUI_GenericContainer{ pDevice, pContext }
{
    m_fStartScaleMul = 1.6f;
    m_fAppearDuration = 0.35f;
    m_fWaitDuration = 1.0f;
}

CUI_StageClear::CUI_StageClear(const CUI_StageClear& Prototype)
    : CUI_GenericContainer(Prototype)
    , m_fStartScaleMul(Prototype.m_fStartScaleMul)
    , m_fAppearDuration(Prototype.m_fAppearDuration)
    , m_fWaitDuration(Prototype.m_fWaitDuration)
{
}

HRESULT CUI_StageClear::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CUI_StageClear::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    Set_Active(false);

    return S_OK;
}

void CUI_StageClear::On_Deserialized()
{
    m_pEffectLines[0] = m_UIPartObjects.find(L"LineEffect_L")->second;
    m_pEffectLines[1] = m_UIPartObjects.find(L"LineEffect_R")->second;
}

HRESULT CUI_StageClear::Ready_Events()
{
    Subscribe_Event(EventTag::StageClear_UIStarted, [this](void* /*pData*/)
        {
            if (m_bIntroTriggered)
                return;
            Set_Active(true);
            m_bIntroTriggered = true;
            Play_Intro(m_fStartScaleMul, m_fAppearDuration, m_fWaitDuration);
        });
    return S_OK;
}

void CUI_StageClear::On_IntroFinished()
{
    for (auto EffectLine : m_pEffectLines)
        EffectLine->Set_Active(false);

 
    CUTSCENE_CAMERA_DESC Cam{};
    Cam.eCam = ECutsceneCam::Cutscene;
    Cam.szTrack = L"Cut4_Camera";
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &Cam);
    m_pGameInstance_Proxy->Publish(EventTag::StageClear_SequenceFinished, nullptr);
    __super::On_IntroFinished();
}

CUI_StageClear* CUI_StageClear::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_StageClear* pInstance = new CUI_StageClear(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_StageClear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CUI_StageClear::Clone(void* pArg)
{
    CUI_StageClear* pInstance = new CUI_StageClear(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_StageClear");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CUI_StageClear::Free()
{
    __super::Free();
}