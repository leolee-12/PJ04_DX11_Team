#include "Dialogue_Director.h"
#include "GameInstance.h"
#include "GameContent_Events.h"

CDialogue_Director::CDialogue_Director(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CDialogue_Director::CDialogue_Director(const CDialogue_Director& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CDialogue_Director::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDialogue_Director::Initialize(void* pArg)
{
    return __super::Initialize(pArg);
}

HRESULT CDialogue_Director::Ready_Events()
{
    Subscribe_Event(EventTag::StageClear_UIStarted, [this](void*)
        {
            if (ESEQ::IDLE != m_eSeq)
                return;

            m_pGameInstance_Proxy->Publish(EventTag::CutFade_Out, nullptr);
            m_eSeq = ESEQ::FADEOUT;
        });

    Subscribe_Event(EventTag::CutFade_OutDone, [this](void*)
        {
            if (ESEQ::FADEOUT != m_eSeq)
                return;

            m_pGameInstance_Proxy->Publish(EventTag::Dialogue_Arrange, nullptr);
            m_eSeq = ESEQ::FADEIN;
        });

    Subscribe_Event(EventTag::CutFade_InDone, [this](void*)
        {
            if (ESEQ::FADEIN != m_eSeq)
                return;

            m_pGameInstance_Proxy->Publish(EventTag::Dialogue_Start, nullptr);
            m_eSeq = ESEQ::TALK;
        });

    Subscribe_Event(EventTag::Dialogue_Finished, [this](void*)
        {
            if (ESEQ::TALK != m_eSeq)
                return;

            m_eSeq = ESEQ::DONE;
            m_pGameInstance_Proxy->Publish(EventTag::FadeOut_Start, nullptr);
        });

    return S_OK;
}

CDialogue_Director* CDialogue_Director::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDialogue_Director* pInstance = new CDialogue_Director(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CDialogue_Director");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CDialogue_Director::Clone(void* pArg)
{
    CDialogue_Director* pInstance = new CDialogue_Director(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CDialogue_Director");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDialogue_Director::Free()
{
    __super::Free();
}