#include "UI_CurtainSequenceBase.h"
#include "GameInstance.h"
#include "UIPartObject.h"
#include "ICurtainPart.h"

CUI_CurtainSequenceBase::CUI_CurtainSequenceBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIContainerObject(pDevice, pContext)
    , m_bStartHidden{ true }
    , m_bAutoStart{ false }
    , m_bLoop{ false }
    , m_fLoopPeriod{ 0.f }
{
}
CUI_CurtainSequenceBase::CUI_CurtainSequenceBase(const CUI_CurtainSequenceBase& Prototype)
    : CUIContainerObject(Prototype)
    , m_bStartHidden{ Prototype.m_bStartHidden }
    , m_bAutoStart{ Prototype.m_bAutoStart }
    , m_bLoop{ Prototype.m_bLoop }
    , m_fLoopPeriod{ Prototype.m_fLoopPeriod }
{
}

HRESULT CUI_CurtainSequenceBase::Initialize_Prototype() { return __super::Initialize_Prototype(); }

HRESULT CUI_CurtainSequenceBase::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_bStarted = false; m_bDone = false; m_fLoopAcc = 0.f;
    if (m_bStartHidden && !m_bAutoStart)
        Set_Active(false);

    return S_OK;
}

void CUI_CurtainSequenceBase::Reset_Containor()
{
    if (!m_bCollected) Collect_Parts();
    for (auto* fp : m_Parts)
        fp->Reset();
}

void CUI_CurtainSequenceBase::On_Deserialized()
{
    __super::On_Deserialized();
    Refresh_Subscription();
}

void CUI_CurtainSequenceBase::Refresh_Subscription()
{
    // 이벤트명이 바뀌었으면(또는 비워졌으면) 기존 구독 해제
    if (m_bSubscribed && m_hTrigger.strEventType != m_strTriggerEvent)
    {
        m_pGameInstance_Proxy->UnSubscribe(m_hTrigger);
        m_bSubscribed = false;
    }

    // 이벤트명이 있고 아직 구독 안 했으면 구독
    if (!m_strTriggerEvent.empty() && !m_bSubscribed)
    {
        m_hTrigger = m_pGameInstance_Proxy->Subscribe(
            m_strTriggerEvent, [this](void*) { Play(); });   // 비활성이어도 콜백은 옴 → Play()가 Active
        m_bSubscribed = true;
    }
}

void CUI_CurtainSequenceBase::Collect_Parts()
{
    m_Parts.clear();
    std::vector<std::pair<_wstring, CUIPartObject*>> ordered;
    Get_UIPartObjectsInOrder(&ordered);
    for (auto& [tag, pPart] : ordered)
        if (auto* fp = dynamic_cast<ICurtainPart*>(pPart))
            m_Parts.push_back(fp);
    m_bCollected = true;
}

void CUI_CurtainSequenceBase::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Priority_Update(fTimeDelta);
}

void CUI_CurtainSequenceBase::Play()
{
    Set_Active(true);
    if (!m_bCollected) Collect_Parts();
    m_bStarted = true;
    m_bDone = false;
    m_fLoopAcc = 0.f;
    for (auto* fp : m_Parts)
        fp->Begin_Delayed();

    On_SequenceStart();
}

void CUI_CurtainSequenceBase::Stop()
{
    m_bStarted = false;
    for (auto* fp : m_Parts) fp->Anim_Stop();
}

void CUI_CurtainSequenceBase::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (!m_bCollected && !m_UIPartObjects.empty())
        Collect_Parts();

    if (m_bAutoStart && !m_bStarted && m_bCollected)
        Play();

    __super::Update(fTimeDelta);

    if (!m_bStarted)
        return;

    // 주기 기반 루프 (LoopPeriod > 0) 끝나는 파트가 없어도 동작
    if (m_bLoop && m_fLoopPeriod > 0.f)
    {
        m_fLoopAcc += fTimeDelta;
        if (m_fLoopAcc >= m_fLoopPeriod)
        {
            Reset_Containor();
            Play();
        }
        return;
    }

    // done 감지 (끝나는 non-loop 파트가 있을 때만)
    if (!m_bDone)
    {
        _bool bAny = false, bAll = true;
        for (auto* fp : m_Parts)
        {
            if (fp->Is_Loop()) continue;
            bAny = true;
            if (!fp->Is_Finished()) { bAll = false; break; }
        }
        if (bAny && bAll)
        {
            m_bDone = true;
            On_SequenceDone();          // 파생별 완료 신호
        }
    }

    // done 기반 루프
    if (m_bDone && m_bLoop)
    {
        Reset_Containor();
        Play();
        m_bDone = false;
    }
}

void CUI_CurtainSequenceBase::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    __super::Late_Update(fTimeDelta);
}

void CUI_CurtainSequenceBase::Free() 
{ 
    if (m_bSubscribed)
    {
        m_pGameInstance_Proxy->UnSubscribe(m_hTrigger);
        m_bSubscribed = false;
    }
    __super::Free(); 
}