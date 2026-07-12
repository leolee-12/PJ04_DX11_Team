#include "SequencePlayer.h"
#include "GameInstance.h"
#include "DataLoader.h"
#include "CutsceneActor.h"
#include "Kirby.h"

CSequencePlayer::CSequencePlayer(CGameInstance_Proxy* pProxy)
    : m_pGameInstance_Proxy{ pProxy }
{
}

HRESULT CSequencePlayer::Load(const _wstring& strFilePath)
{
    string strContent{};
    if (FAILED(CDataLoader::Read_Json(strFilePath.c_str(), &strContent)))
        return E_FAIL;

    json j{};
    try { j = json::parse(strContent); }
    catch (json::exception&) { return E_FAIL; }

    m_Steps.clear();

    for (auto& jStep : j["Steps"])
    {
        SEQUENCE_STEP tStep{};
        const string strType = jStep.value("type", "");

        if (strType == "wait")  tStep.eType = ESTEP::WAIT;
        else if (strType == "event") tStep.eType = ESTEP::EVENT;
        else if (strType == "warp")  tStep.eType = ESTEP::WARP;
        else if (strType == "anim")  tStep.eType = ESTEP::ANIM;
        else if (strType == "say")   tStep.eType = ESTEP::SAY;
        else
        {
            OutputDebugStringA(("[SequencePlayer] unknown step type: " + strType + "\n").c_str());
            continue;
        }

        tStep.fDuration = jStep.value("duration", 0.f);
        tStep.strTag = StrToWstr(jStep.value("tag", ""));
        tStep.strActor = StrToWstr(jStep.value("actor", ""));
        tStep.strAnchor = StrToWstr(jStep.value("anchor", ""));
        if (jStep.contains("wait") && jStep["wait"].is_number())
            tStep.fDuration = jStep["wait"].get<_float>();
        else
            tStep.fDuration = jStep.value("duration", 0.f);

        tStep.strTag = StrToWstr(jStep.value("tag", ""));
        tStep.strActor = StrToWstr(jStep.value("actor", ""));
        tStep.strAnchor = StrToWstr(jStep.value("anchor", ""));
        tStep.strClip = StrToWstr(jStep.value("clip", ""));
        tStep.strSpeaker = StrToWstr(jStep.value("name", ""));
        if (jStep.contains("text"))
        {
            auto& jText = jStep["text"];
            if (jText.is_array())
            {
                const size_t iCount = min<size_t>(jText.size(), 3);
                for (size_t k = 0; k < iCount; ++k)
                    tStep.strLines[k] = StrToWstr(jText[k].get<string>());
            }
            else
                tStep.strLines[0] = StrToWstr(jText.get<string>());
        }
        tStep.bLoop = jStep.value("loop", false);

        m_Steps.push_back(tStep);
    }

    return m_Steps.empty() ? E_FAIL : S_OK;
}

void CSequencePlayer::Bind_Actor(const _wstring& strName, CGameObject* pActor)
{
    if (pActor) m_Actors[strName] = pActor;
}

void CSequencePlayer::Bind_Anchor(const _wstring& strName, _fmatrix AnchorWorld)
{
    _float4x4 Out{};
    XMStoreFloat4x4(&Out, AnchorWorld);
    m_Anchors[strName] = Out;
}

void CSequencePlayer::Play()
{
    if (m_Steps.empty())
        return;

    m_iCurStep = 0;
    m_fStepTime = 0.f;
    m_bPlaying = true;
    m_bFinished = false;
    Enter_Step(m_Steps[0]);
}

void CSequencePlayer::Update(_float fTimeDelta)
{
    if (!m_bPlaying)
        return;

    m_fStepTime += fTimeDelta;

    while (m_bPlaying && Is_StepDone(m_Steps[m_iCurStep]))
        Advance();
}

void CSequencePlayer::Enter_Step(const SEQUENCE_STEP& tStep)
{
    switch (tStep.eType)
    {
        case ESTEP::EVENT:
            m_pGameInstance_Proxy->Publish(tStep.strTag.c_str(), nullptr);
            break;
        case ESTEP::WARP:
            Execute_Warp(tStep);
            break;
        case ESTEP::ANIM:
            Execute_Anim(tStep);
            break;
        case ESTEP::SAY:
            Execute_Say(tStep);
            break;
        default:
            break;
    }
}

_bool CSequencePlayer::Is_StepDone(const SEQUENCE_STEP& tStep)
{
    switch (tStep.eType)
    {
        case ESTEP::SAY:
            return m_bSayDone;    // UI 좌클릭이 Dialogue_SayDone -> 어레인저 중계로 세워줌

        default:
            return m_fStepTime >= tStep.fDuration;
    }
}

void CSequencePlayer::Advance()
{
    ++m_iCurStep;
    m_fStepTime = 0.f;

    if (m_iCurStep >= m_Steps.size())
    {
        m_bPlaying = false;
        m_bFinished = true;
        return;
    }
    Enter_Step(m_Steps[m_iCurStep]);
}

void CSequencePlayer::Execute_Warp(const SEQUENCE_STEP& tStep)
{
    auto itActor = m_Actors.find(tStep.strActor);
    auto itAnchor = m_Anchors.find(tStep.strAnchor);
    if (itActor == m_Actors.end() || itAnchor == m_Anchors.end())
    {
        OutputDebugStringW(L"[SequencePlayer] warp: actor or anchor not bound\n");
        return;
    }

    _matrix Anchor = XMLoadFloat4x4(&itAnchor->second);
    _float3 vPos{}, vLook{};
    XMStoreFloat3(&vPos, Anchor.r[3]);
    XMStoreFloat3(&vLook, XMVector3Normalize(Anchor.r[2]));

    if (dynamic_cast<CKirby*>(itActor->second))
    {
        SEQUENCE_KIRBY_WARP_DESC Desc{};
        Desc.vPosition = vPos;
        Desc.vLook = vLook;
        m_pGameInstance_Proxy->Publish(EventTag::Sequence_KirbyWarp, &Desc);
        return;
    }

    CTransform* pTf = itActor->second->Get_Transform();
    pTf->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vPos), 1.f));
    pTf->LookAt(pTf->Get_State(STATE::POSITION) + XMLoadFloat3(&vLook));
}

void CSequencePlayer::Execute_Anim(const SEQUENCE_STEP& tStep)
{
    auto it = m_Actors.find(tStep.strActor);
    if (it == m_Actors.end())
    {
        OutputDebugStringW(L"[SequencePlayer] anim: actor not bound\n");
        return;
    }

    // 커비: 이벤트 계약으로 위임. 상태 처리 방식은 커비 측 자유
    if (dynamic_cast<CKirby*>(it->second))
    {
        SEQUENCE_KIRBY_ANIM_DESC Desc{};
        Desc.strClip = WstrToStr(tStep.strClip);
        Desc.bLoop = tStep.bLoop;
        m_pGameInstance_Proxy->Publish(EventTag::Sequence_KirbyAnim, &Desc);
        return;
    }

    // NPC: CutsceneActor 직접 재생
    if (auto pActor = dynamic_cast<CCutsceneActor*>(it->second))
    {
        pActor->Play(WstrToStr(tStep.strClip).c_str(), tStep.bLoop);
        return;
    }

    OutputDebugStringW(L"[SequencePlayer] anim: actor type not supported\n");
}

void CSequencePlayer::Execute_Say(const SEQUENCE_STEP& tStep)
{
    m_bSayDone = false;

    DIALOGUE_SAY_DESC Desc{};
    Desc.strSpeaker = tStep.strSpeaker;
    for (_uint i = 0; i < 3; ++i)
        Desc.strLines[i] = tStep.strLines[i];

    m_pGameInstance_Proxy->Publish(EventTag::Dialogue_Say, &Desc);
}

CSequencePlayer* CSequencePlayer::Create(CGameInstance_Proxy* pProxy)
{
    return new CSequencePlayer(pProxy);
}

void CSequencePlayer::Free()
{
    __super::Free();
}