#include "Animator.h"
#include "Model.h"
#include <fstream>

CAnimator::CAnimator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext } {
}

CAnimator::CAnimator(const CAnimator& Prototype)
    : CComponent(Prototype), m_Tracks{ Prototype.m_Tracks } {
}

HRESULT CAnimator::Initialize(void* pArg)
{
    if (pArg == nullptr)
        return E_FAIL;

    ANIMATOR_DESC* pDesc = static_cast<ANIMATOR_DESC*>(pArg);
    m_pModel = pDesc->pModel;
    Safe_AddRef(m_pModel);

    if (!pDesc->strDataFile.empty())
        Load_FromFile(pDesc->strDataFile);

    return S_OK;
}

// ── 재생 제어 ──
void CAnimator::Play(const string& strAnimName, _bool bLoop, _bool bRestart, _float fBlend)
{
    if (nullptr == m_pModel)
        return;

    _int iIndex = m_pModel->Get_AnimationIndex(strAnimName);

    if (iIndex < 0)
        return;

    m_pModel->Set_AnimationIndex((_uint)iIndex, bLoop, bRestart, fBlend);

    m_bFinished = false;
}

void CAnimator::Seek(_float fProgress)
{
    if (nullptr == m_pModel)
        return;

    m_pModel->Seek_Animation(fProgress);

    m_fPrevProgress = fProgress;   // 스크럽 직후 이벤트 중복발화 방지
}

const string& CAnimator::Get_CurrentAnimName() const
{
    static const string strEmpty = {};
    return m_pModel ? m_pModel->Get_CurrentAnimName() : strEmpty;
}

_float CAnimator::Get_Progress() const
{
    return m_pModel ? m_pModel->Get_CurrentAnimProgress() : 0.f;
}

// ── Update: 재생 + 이벤트 판정 ──
void CAnimator::Update(_float fTimeDelta)
{
    if (nullptr == m_pModel)
        return;

    if (!m_bPaused)
        m_bFinished = m_pModel->Play_Animation(fTimeDelta);   // ★ 애니메이터가 단독 구동

    const string& strCur = m_pModel->Get_CurrentAnimName();
    _float        fCur = m_pModel->Get_CurrentAnimProgress();

    if (strCur != m_strPrevAnimName)
    {
        auto itPrev = m_Tracks.find(m_strPrevAnimName);
        Reset_RuntimeState(itPrev != m_Tracks.end() ? &itPrev->second : nullptr);
        m_strPrevAnimName = strCur;
        m_fPrevProgress = 0.f;
    }

    auto it = m_Tracks.find(strCur);
    if (it != m_Tracks.end())
    {
        auto& track = it->second;
        if (fCur < m_fPrevProgress)                 // 루프 wrap
        {
            Fire_Point(track.Events, m_fPrevProgress, 1.0f);
            Fire_Point(track.Events, -0.0001f, fCur);
        }
        else
        {
            Fire_Point(track.Events, m_fPrevProgress, fCur);
        }
        Process_Range(track, fCur);
    }

    m_fPrevProgress = fCur;
}

void CAnimator::Fire_Point(const vector<ANIM_EVENT>& events, _float lo, _float hi)
{
    for (auto& e : events)
    {
        if (e.bIsRange) continue;
        if (e.fTriggerProgress > lo && e.fTriggerProgress <= hi)
            if (m_Callback) m_Callback(e, ANIM_EVENT_PHASE::POINT);
    }
}

void CAnimator::Process_Range(ANIM_EVENT_TRACK& track, _float fCur)
{
    for (auto& e : track.Events)
    {
        if (!e.bIsRange) continue;
        _bool inside = (fCur >= e.fTriggerProgress && fCur <= e.fEndProgress);
        if (inside && !e.bActive) { e.bActive = true;  if (m_Callback) m_Callback(e, ANIM_EVENT_PHASE::BEGIN); }
        if (inside) { if (m_Callback) m_Callback(e, ANIM_EVENT_PHASE::TICK); }
        if (!inside && e.bActive) { e.bActive = false; if (m_Callback) m_Callback(e, ANIM_EVENT_PHASE::END); }
    }
}

void CAnimator::Reset_RuntimeState(ANIM_EVENT_TRACK* pTrack)
{
    if (!pTrack) return;
    for (auto& e : pTrack->Events) e.bActive = false;
}

// ── 에디터 데이터 ──
ANIM_EVENT_TRACK& CAnimator::Get_Track(const string& strAnimName)
{
    auto& track = m_Tracks[strAnimName];
    if (track.strAnimName.empty()) track.strAnimName = strAnimName;
    return track;
}

void CAnimator::Sort_Track(const string& strAnimName)
{
    auto it = m_Tracks.find(strAnimName);
    if (it == m_Tracks.end()) return;
    sort(it->second.Events.begin(), it->second.Events.end(),
        [](const ANIM_EVENT& a, const ANIM_EVENT& b) { return a.fTriggerProgress < b.fTriggerProgress; });
}

json CAnimator::Serialize() const
{
    json j;
    for (auto& [name, track] : m_Tracks)
    {
        json jt = json::array();
        for (auto& e : track.Events)
            jt.push_back({ {"type",e.iEventType},{"p",e.fTriggerProgress},{"range",e.bIsRange},
                           {"end",e.fEndProgress},{"param",e.strParam},{"i",e.iIntParam},
                           {"off",{e.vOffset.x,e.vOffset.y,e.vOffset.z}} });
        j[name] = jt;
    }
    return j;
}

void CAnimator::Deserialize_Internal(const json& j)
{
    m_Tracks.clear();
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        ANIM_EVENT_TRACK track; track.strAnimName = it.key();
        for (auto& je : it.value())
        {
            ANIM_EVENT e;
            e.iEventType = je.value("type", 0);
            e.fTriggerProgress = je.value("p", 0.f);
            e.bIsRange = je.value("range", false);
            e.fEndProgress = je.value("end", 0.f);
            e.strParam = je.value("param", string());
            e.iIntParam = je.value("i", 0);
            if (je.contains("off") && je["off"].size() == 3)
                e.vOffset = { je["off"][0], je["off"][1], je["off"][2] };
            track.Events.push_back(e);
        }
        m_Tracks[track.strAnimName] = move(track);
    }
}

HRESULT CAnimator::Load_FromFile(const wstring& strPath)
{
    ifstream fin(strPath); if (!fin.is_open()) return E_FAIL;
    json j; fin >> j; fin.close(); Deserialize(j); return S_OK;
}

HRESULT CAnimator::Save_ToFile(const wstring& strPath) const
{
    ofstream fout(strPath); if (!fout.is_open()) return E_FAIL;
    fout << Serialize().dump(2); fout.close(); return S_OK;
}

CAnimator* CAnimator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CAnimator(pDevice, pContext);
}

CComponent* CAnimator::Clone(void* pArg)
{
    CAnimator* pInstance = new CAnimator(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAnimator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAnimator::Free()
{
    __super::Free();
    Safe_Release(m_pModel);
}