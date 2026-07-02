#include "Animator.h"
#include "Model.h"
#include <fstream>

CAnimator::CAnimator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
    , m_fPlaySpeed{ 1.f }
    , m_fBlendDuration{ 0.2f }
{
}

CAnimator::CAnimator(const CAnimator& Prototype)
    : CComponent(Prototype)
    , m_Tracks{ Prototype.m_Tracks }
    , m_fPlaySpeed {Prototype.m_fPlaySpeed}
    , m_fBlendDuration{Prototype.m_fBlendDuration}
{
}

HRESULT CAnimator::Initialize(void* pArg)
{
    if (pArg == nullptr)
        return E_FAIL;

    ANIMATOR_DESC* pDesc = static_cast<ANIMATOR_DESC*>(pArg);
    m_pModel = pDesc->pModel;
    Safe_AddRef(m_pModel);

    m_strDataFilePath = !pDesc->strDataFile.empty() ? pDesc->strDataFile : Make_DefaultDataFilePath();

    if (!pDesc->strDataFile.empty())
        if (FAILED(Load_FromFile(m_strDataFilePath)))
            MSG_BOX("NotFound : AnimEvents.json");

    return S_OK;
}

// ── 재생 제어 ──
void CAnimator::Play(const string& strAnimName, _bool bLoop, _bool bRestart, _float fBlend, _float fSpeed, _bool bClearMask)
{
    if (nullptr == m_pModel)
        return;

    _int iIndex = m_pModel->Get_AnimationIndex(strAnimName);

    if (iIndex < 0)
        return;

    m_fBlendDuration = fBlend;

    //m_pModel->Set_AnimationIndex((_uint)iIndex, bLoop, bRestart, m_fBlendDuration);
    m_fPlaySpeed = fSpeed;
    m_bFinished = false;

    m_PlayQueue.clear();
    Start_Clip({ strAnimName, bLoop, bRestart, fBlend, fSpeed, bClearMask });
}

void CAnimator::Play(const ANI_PLAY_INFO* tAniInfo)
{
    if (nullptr == tAniInfo)
        return;

    m_PlayQueue.clear();

    Start_Clip(*tAniInfo);
}

void CAnimator::Start_Clip(const ANI_PLAY_INFO& Info)
{
    if (nullptr == m_pModel)
        return;

    _int iIndex = m_pModel->Get_AnimationIndex(Info.strAniName);
    if (iIndex < 0)
        return;

    const _bool bRestartSameClip = Info.bRestart && m_pModel->Get_CurrentAnimName() == Info.strAniName;

    if (bRestartSameClip)
    {
        auto it = m_Tracks.find(Info.strAniName);

        if (it != m_Tracks.end())
            Reset_RuntimeState(&it->second);

        m_fPrevProgress = -0.0001f;
    }

    m_fBlendDuration = Info.fBlend;
    m_pModel->Set_AnimationIndex(static_cast<_uint>(iIndex), Info.bLoop, Info.bRestart, m_fBlendDuration);

    m_fPlaySpeed = Info.fSpeed;
    m_bFinished = false;
    m_bCurLoop = Info.bLoop;
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

void CAnimator::SpinByProgress(const _char* szBone, _float fTurns, _fvector vAxis, _uint iSlot)
{
    _float fProgress = Get_LayerProgress(iSlot);
    _float fAngle = 360.f * fTurns * fProgress;
    SetBoneRotation(szBone, fAngle, vAxis);
}

void CAnimator::TiltByProgress(const _char* szBone, _float fPeakDeg, _fvector vAxis, _uint iSlot)
{
    _float fProgress = Get_LayerProgress(iSlot);
    _float fAngle = fPeakDeg * sinf(XM_PI * fProgress);
    SetBoneRotation(szBone, fAngle, vAxis);
}

void CAnimator::Set_Mask(const _char* szClip, const _char* szRootBone, _bool bLoop, _float fMaskTarget, _float fMaskBlendTime)
{
    const _char* Roots[] = { szRootBone };
    Set_Mask(szClip, Roots, 1, bLoop, fMaskTarget, fMaskBlendTime);
}

void CAnimator::Set_Mask(const _char* szClip, const _char* const* pRoots, _uint iRootCount, _bool bLoop, _float fMaskTarget, _float fMaskBlendTime)
{
    // 임시 브릿지 역할
    LAYER_PLAY_INFO tInfo;
    tInfo.iSlot = 1;
    tInfo.tAnim.strAniName = (szClip != nullptr) ? szClip : "";
    tInfo.tAnim.bLoop = bLoop;
    tInfo.fTargetWeight = fMaskTarget;
    tInfo.fWeightBlend = fMaskBlendTime;

    tInfo.Roots.reserve(iRootCount);
    for (_uint i = 0; i < iRootCount; ++i)
    {
        const _char* sz = pRoots ? pRoots[i] : nullptr;
        if (sz != nullptr)
            tInfo.Roots.push_back(sz);
    }

    Apply_Overlay(tInfo);
}

void CAnimator::Clear_Mask(_float fMaskBlendTime)
{
    Clear_Overlay(1, fMaskBlendTime);
}

void CAnimator::Apply_Overlay(const LAYER_PLAY_INFO& tInfo)
{
    if (nullptr == m_pModel)
        return;

    const _uint iSlot = tInfo.iSlot;
    if (iSlot == 0 || iSlot >= MAX_LAYERS)      // 0번은 Base 
        return;

    const _string& strClip = tInfo.tAnim.strAniName;

    if (strClip.empty())
        return;

    const _int iAnimIndex = m_pModel->Get_AnimationIndex(strClip);
    if (iAnimIndex < 0)      // 없는 클립 무시
        return;

    LAYER& Layer = m_Layers[iSlot];

    const _bool bActive = !Layer.strClip.empty();                       // 활성화 되었는지
    const _bool bSameClip = bActive && (Layer.strClip == strClip);       // 같은 클립인지

    if (!bActive)
    {
        // 비활성 슬롯을 활성화 시키겠다라는 의도 0 -> Weight-In
        Layer.fWeight = 0.f;
        Layer.strClip = strClip;
        Layer.iAnimIndex = iAnimIndex;
        Layer.bLoop = tInfo.tAnim.bLoop;
        Layer.bFinished = false;
        Layer.fLocalTime = 0.f;
        Layer.KeyFrameCursors.clear();
        Layer.bClipBlending = false;
        Layer.iPrevAnimIndex = -1;
        Layer.PrevCursors.clear();
        m_pModel->Build_MaskBones(tInfo.Roots, Layer.MaskBones);     
    }
    else if (!bSameClip)
    {
        // 현재 클립을 나가는 쪽으로 이동
        Layer.iPrevAnimIndex = Layer.iAnimIndex;
        Layer.fPrevLocalTime = Layer.fLocalTime;
        Layer.PrevCursors = Layer.KeyFrameCursors;
        Layer.bPrevLoop = Layer.bLoop;
        Layer.bClipBlending = true;
        
        // 이미 활성화 되어있는 슬롯
        Layer.strClip = strClip;
        Layer.iAnimIndex = iAnimIndex;
        Layer.bLoop = tInfo.tAnim.bLoop;
        Layer.bFinished = false;
        Layer.fLocalTime = 0.f;
        Layer.KeyFrameCursors.clear();

        Layer.fClipBlend = tInfo.tAnim.fBlend;   
        Layer.fClipBlendElapsed = 0.f;          // 블렌드 시간 초기화
        m_pModel->Build_MaskBones(tInfo.Roots, Layer.MaskBones);
    }
    else if (tInfo.tAnim.bRestart)      // 같은 클립 + bRestart true라면 처음부터 재생
    {
        Layer.fLocalTime = 0.f;
        Layer.KeyFrameCursors.clear();
        Layer.bFinished = false;
    }

    Layer.fTarget       = tInfo.fTargetWeight;
    Layer.fWeightBlend  = tInfo.fWeightBlend;
    Layer.fSpeed        = tInfo.tAnim.fSpeed;       
}

void CAnimator::Clear_Overlay(_uint iSlot, _float fWeightBlend)
{
    if (iSlot == 0 || iSlot >= MAX_LAYERS)
        return;

    LAYER& Layer = m_Layers[iSlot];
    if (Layer.strClip.empty())      // 이미 비활성된 슬롯
        return;

    Layer.fTarget = 0.f;

    if (fWeightBlend <= 0.f)
    {
        // 즉시 정리
        Layer.fWeight = 0.f;
        Layer.fWeightBlend = 0.f;
        Layer.strClip.clear();
        Layer.iAnimIndex = -1;
        Layer.bFinished = false;
        Layer.fLocalTime = 0.f;
        Layer.KeyFrameCursors.clear();
        Layer.MaskBones.clear();
        Layer.bClipBlending = false;
        Layer.iPrevAnimIndex = -1;
        Layer.PrevCursors.clear();
        return;
    }

    Layer.fWeightBlend = fWeightBlend;       // Fade Out -> Update에서 Weight 0 도달 시 정리
}

_bool CAnimator::Is_Overlay_Finished(_uint iSlot) const
{
    if (iSlot >= MAX_LAYERS)
        return false;

    if (iSlot == 0)
        return Is_Finished();

    return m_Layers[iSlot].bFinished;       // TODO : Model에서 오버레이 클립 종료 신호 연결
}

void CAnimator::Enqueue(const ANI_PLAY_INFO& info)
{
    m_PlayQueue.push_back(info);
}

void CAnimator::SetBoneRotation(const _char* szBone, _float fAngleDeg, _fvector vAxis)
{
    m_strRotBone = szBone;
    m_fRotAngle = fAngleDeg;
    XMStoreFloat4(&m_vRotAxis, vAxis);
    m_bHasRotReq = true;
}

_bool CAnimator::Has_Bone(const _char* szBone) const
{
    if (m_pModel == nullptr)
        return false; 

    return m_pModel->Get_BoneIndex(szBone) >= 0;
}

// ── Update: 재생 + 이벤트 판정 ──
void CAnimator::Update(_float fTimeDelta)
{
    if (nullptr == m_pModel)
        return;
    
    for (_uint iSlot = 1; iSlot < MAX_LAYERS; ++iSlot)
    {
        LAYER& Layer = m_Layers[iSlot];
        if (Layer.strClip.empty() && Layer.fWeight <= 0.f)
            continue;                                           // 미사용 슬롯은 스킵

        _float fStep = (Layer.fWeightBlend > 0.f) ? (fTimeDelta / Layer.fWeightBlend) : 1.f;
        if (Layer.fWeight < Layer.fTarget)
            Layer.fWeight = min(Layer.fWeight + fStep, Layer.fTarget);
        else
            Layer.fWeight = max(Layer.fWeight - fStep, Layer.fTarget);

        // 페이드 아웃 완료 -> 슬롯 비활성화
        if (!Layer.strClip.empty() && Layer.fTarget <= 0.f && Layer.fWeight <= 0.f)
        {
            Layer.strClip.clear();
            Layer.bFinished = false;
            Layer.iAnimIndex = -1;
            Layer.fLocalTime = 0.f;
            Layer.KeyFrameCursors.clear();
            Layer.MaskBones.clear();
            Layer.bClipBlending = false;
            Layer.iPrevAnimIndex = -1;
            Layer.PrevCursors.clear();
        }
    }

    if (!m_bPaused)
    {
        m_bFinished = m_pModel->Update_Base(fTimeDelta, m_fPlaySpeed);              // 베이스 재생

        for (_uint iSlot = 1; iSlot < MAX_LAYERS; ++iSlot)
        {
            LAYER& Layer = m_Layers[iSlot];
            if (!Layer.strClip.empty() && Layer.fWeight > 0.f)
                Layer.bFinished = m_pModel->Apply_Mask(Layer, fTimeDelta);          // Base 위에 블렌드
        }

        if (m_bHasRotReq)
        {
            m_pModel->RotateBone(m_strRotBone.c_str(), m_fRotAngle, XMLoadFloat4(&m_vRotAxis));
            m_bHasRotReq = false;
        }

        m_pModel->Update_Combined();                                                // 모든 레이어 쌓인 뒤에 계층 결합 1회
    }

    const string& strCur = m_pModel->Get_CurrentAnimName();
    _float        fCur = m_pModel->Get_CurrentAnimProgress();

    if (strCur != m_strPrevAnimName)
    {
        auto itPrev = m_Tracks.find(m_strPrevAnimName);
        Reset_RuntimeState(itPrev != m_Tracks.end() ? &itPrev->second : nullptr);
        m_strPrevAnimName = strCur;
        m_fPrevProgress = 0.f;
        m_fPrevProgress = -0.0001f;
    }

    auto it = m_Tracks.find(strCur);
    if (it != m_Tracks.end())
    {
        auto& track = it->second;
        if (fCur < m_fPrevProgress)                 
        {
            Fire_Point(track.Events, m_fPrevProgress, 1.0f);
            if (m_bCurLoop)                      
                for (auto& e : track.Events) e.bFired = false;
            Fire_Point(track.Events, -0.0001f, fCur);
        }
        else
            Fire_Point(track.Events, m_fPrevProgress, fCur);

        Process_Range(track, fCur);
    }

    m_fPrevProgress = fCur;

    if (m_bFinished && !m_PlayQueue.empty())
    {
        ANI_PLAY_INFO next = m_PlayQueue.front();
        m_PlayQueue.pop_front();
        Start_Clip(next);           // Play 아님, 큐 안 비움
    }
}

_float CAnimator::Get_LayerProgress(_uint iSlot) const
{
    if (nullptr == m_pModel)
        return 0.f;

    if (iSlot == 0)
        return m_pModel->Get_CurrentAnimProgress();

    const LAYER& L = m_Layers[iSlot];
    if (L.strClip.empty() || L.iAnimIndex < 0)
        return 0.f;

    _float dur = m_pModel->Get_AnimationDuration(L.iAnimIndex);

    return (dur > 0.f) ? (L.fLocalTime / dur) : 0.f;
}

void CAnimator::Fire_Point(vector<ANIM_EVENT>& events, _float lo, _float hi)
{
    for (auto& e : events)
    {
        if (e.bIsRange) continue;
        if (e.fTriggerProgress > lo && e.fTriggerProgress <= hi)
        {
            if (e.bFired) continue;
            e.bFired = true;
            if (m_Callback) m_Callback(e, ANIM_EVENT_PHASE::POINT);
        }
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
    for (auto& e : pTrack->Events) 
    { 
        e.bActive = false;
        e.bFired = false; 
    }
}

_wstring CAnimator::Make_DefaultDataFilePath() const
{
    if (nullptr == m_pModel)
        return L"";

    const _wstring& strModelPath = m_pModel->Get_ModelPath();
    if (strModelPath.empty())
        return L"";

    filesystem::path ModelPath(strModelPath);
    filesystem::path EventPath = ModelPath.parent_path() / (ModelPath.stem().wstring() + L"_AnimEvents.json");

    return EventPath.wstring();
}

_wstring CAnimator::Resolve_DataFilePath(const _wstring& strPath) const
{
    if (!strPath.empty())
        return strPath;

    if (!m_strDataFilePath.empty())
        return m_strDataFilePath;

    return Make_DefaultDataFilePath();
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
    const _wstring strResolvedPath = Resolve_DataFilePath(strPath);
    if (strResolvedPath.empty())
        return E_FAIL;

    ifstream fin(strResolvedPath); 
    if (!fin.is_open()) 
        return E_FAIL;

    json j;
    fin >> j;
    fin.close(); 

    Deserialize(j); 
    m_strDataFilePath = strResolvedPath;

    return S_OK;
}

HRESULT CAnimator::Save_ToFile(const wstring& strPath)
{
    const _wstring strResolvedPath = Resolve_DataFilePath(strPath);
    if (strResolvedPath.empty())
        return E_FAIL;

    ofstream fout(strResolvedPath);
    if (!fout.is_open()) 
        return E_FAIL;

    fout << Serialize().dump(2); 
    fout.close(); 

    m_strDataFilePath = strResolvedPath;

    return S_OK;
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