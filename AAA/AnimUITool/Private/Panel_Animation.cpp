#include "Panel_Animation.h"
#include "Panel_Manager.h"
#include "Preview_Actor.h"
#include "Model.h"
#include "Animator.h"
#include "imgui.h"
#include "GameContent_AnimEvents.h"

CPanel_Animation::CPanel_Animation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel{ pDevice, pContext }
{
	strcpy_s(m_szName, "Animation");
}

void CPanel_Animation::Render() 
{
    ImGui::Begin(m_szName);

    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    if (!ctx.Valid()) { ImGui::TextDisabled("No preview model."); ImGui::End(); return; }

    if (m_pPrevActor != ctx.pActor) 
    { 
        m_pPrevActor = ctx.pActor;
        m_iPrevClip = -1;

        if (ctx.pActor && !ctx.strModelPath.empty())
        {
            namespace fs = filesystem;
            fs::path mp(ctx.strModelPath);
            fs::path jp = mp.parent_path() / (mp.stem().wstring() + L"_anim_events.json");

            std::error_code ec;
            fs::path rel = fs::relative(jp, fs::current_path(), ec);   
            std::wstring out = (!ec && !rel.empty()) ? rel.wstring() : jp.wstring();

            std::string js(out.begin(), out.end());
            strcpy_s(m_szEventPath, js.c_str());
        }
    }

    CModel* pModel = ctx.pModel;
    CAnimator* pAnim = ctx.pAnimator;

    if (pModel->Get_NumAnimations() == 0)          
    {
        ImGui::TextDisabled("No animation clips (model-only .ysh).");
        ImGui::TextDisabled("Use a baked .ysh (model + motion) to preview animation.");
        ImGui::End();
        return;
    }

    const _uint iMax = pModel->Get_MaxAnimationIndex();
    if (ctx.iClip > (_int)iMax) ctx.iClip = 0;

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space, false))
        ctx.bPlaying = !ctx.bPlaying;

    string strName = pModel->Get_AnimationName((_uint)ctx.iClip);

    ImGui::Text("Animation : %u", pModel->Get_NumAnimations());

    ImGui::SetNextItemWidth(260.f);
    if (ImGui::BeginCombo("Clip", strName.c_str()))
    {
        for (_uint i = 0; i <= iMax; ++i)
            if (ImGui::Selectable(pModel->Get_AnimationName(i).c_str(), (_uint)ctx.iClip == i))
                ctx.iClip = (_int)i;
        ImGui::EndCombo();
    }

    if (ctx.iClip != m_iPrevClip)
    {
        pAnim->Play(strName, ctx.bLoop, true, 0.f);
        m_iPrevClip = ctx.iClip;
        ctx.fProgress = 0.f;
    }

    if (ImGui::Button(ctx.bPlaying ? " Stop " : " Play ")) ctx.bPlaying = !ctx.bPlaying;
    ImGui::SameLine();
    ImGui::Checkbox("Loop", &ctx.bLoop);

    ImGui::SetNextItemWidth(-1.f);
    if (ctx.bPlaying)
    {
        pAnim->Resume();
        pAnim->Play(strName, ctx.bLoop, false, 0.f);     
        pAnim->Update(ImGui::GetIO().DeltaTime);          
        ctx.fProgress = pAnim->Get_Progress();
        ImGui::SliderFloat("##timeline", &ctx.fProgress, 0.f, 1.f, "%.3f");
    }
    else
    {
        pAnim->Pause();
        if (ImGui::SliderFloat("##timeline", &ctx.fProgress, 0.f, 1.f, "%.3f"))
            pAnim->Seek(ctx.fProgress);
    }

    Render_EventTimeline();

    ImGui::End();
}

void CPanel_Animation::Render_EventTimeline()
{
    ANIM_CONTEXT& ctx = m_pPanel_Manager->Get_Context();
    CModel* pModel = ctx.pModel;
    CAnimator* pAnim = ctx.pAnimator;

    if (!pModel || !pAnim)
        return;

    const _uint iNumAnims = pModel->Get_NumAnimations();
    if (iNumAnims == 0)
        return;

    if (ctx.iClip < 0 || ctx.iClip >= (_int)iNumAnims)
        ctx.iClip = 0;

    const std::string strName = pModel->Get_AnimationName((_uint)ctx.iClip);
    ANIM_EVENT_TRACK& track = pAnim->Get_Track(strName);

    ImGui::Separator();
    ImGui::Text("Events (%zu)  -  drag marker to move, click empty to scrub-select", track.Events.size());

    // ── 마커 스트립 ──
    const float  fWidth = ImGui::GetContentRegionAvail().x;
    const float  fHeight = 20.f;
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##evstrip", ImVec2(fWidth, fHeight));
    const ImVec2 p1(p0.x + fWidth, p0.y + fHeight);
    const float  fMid = (p0.y + p1.y) * 0.5f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p0, p1, IM_COL32(35, 35, 35, 255));

    auto X = [&](float prog) { return p0.x + prog * fWidth; };

    // 플레이헤드
    dl->AddLine(ImVec2(X(ctx.fProgress), p0.y), ImVec2(X(ctx.fProgress), p1.y), IM_COL32(255, 255, 255, 180), 1.5f);

    // 입력: 누른 순간 가장 가까운 마커 선택(8px), 없으면 해제 / 드래그로 이동
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    float mProg = (fWidth > 0.f) ? (mouse.x - p0.x) / fWidth : 0.f;
    if (mProg < 0.f) mProg = 0.f; else if (mProg > 1.f) mProg = 1.f;

    if (ImGui::IsItemActivated())
    {
        m_iSelEvent = -1;
        float fBest = 8.f;
        for (int i = 0; i < (int)track.Events.size(); ++i)
        {
            float d = fabsf(X(track.Events[i].fTriggerProgress) - mouse.x);
            if (d < fBest) { fBest = d; m_iSelEvent = i; }
        }
    }
    if (ImGui::IsItemActive() && m_iSelEvent >= 0 && m_iSelEvent < (int)track.Events.size())
        track.Events[m_iSelEvent].fTriggerProgress = mProg;   // 드래그 이동

    // 마커 그리기
    for (int i = 0; i < (int)track.Events.size(); ++i)
    {
        const ANIM_EVENT& e = track.Events[i];
        const bool  bSel = (i == m_iSelEvent);
        const ImU32 col = bSel ? IM_COL32(255, 200, 0, 255) : IM_COL32(90, 170, 255, 255);
        if (e.bIsRange)
            dl->AddRectFilled(ImVec2(X(e.fTriggerProgress), p0.y + 3), ImVec2(X(e.fEndProgress), p1.y - 3),
                bSel ? IM_COL32(255, 200, 0, 70) : IM_COL32(90, 170, 255, 70));
        dl->AddCircleFilled(ImVec2(X(e.fTriggerProgress), fMid), bSel ? 6.f : 4.f, col);
    }

    // ── 추가 / 저장 / 로드 ──
    if (ImGui::Button("+ Add Event"))
    {
        track.Events.push_back({ (int)Client::EANIM_EVENT::Fx, ctx.fProgress });
        m_iSelEvent = (int)track.Events.size() - 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        pAnim->Sort_Track(strName);
        m_iSelEvent = -1;

        std::string s(m_szEventPath);
        std::wstring ws(s.begin(), s.end());

        if (SUCCEEDED(pAnim->Save_ToFile(ws)))
            Log_Info("Saved anim events: " + s);
        else
            Log_Error("Failed to save anim events: " + s);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        std::string s(m_szEventPath);
        std::wstring ws(s.begin(), s.end());

        if (SUCCEEDED(pAnim->Load_FromFile(ws)))
            Log_Info("Loaded anim events: " + s);
        else
            Log_Error("Failed to load anim events: " + s);

        m_iSelEvent = -1;
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputText("##json", m_szEventPath, sizeof(m_szEventPath));

    // ── 선택 이벤트 편집 ──
    if (m_iSelEvent >= 0 && m_iSelEvent < (int)track.Events.size())
    {
        ImGui::Separator();
        ANIM_EVENT& e = track.Events[m_iSelEvent];

        const char* szCur = "None";
        for (auto& [v, n] : Client::g_AnimEventNames) if ((int)v == e.iEventType) szCur = n;
        if (ImGui::BeginCombo("Type", szCur))
        {
            for (auto& [v, n] : Client::g_AnimEventNames)
                if (ImGui::Selectable(n, (int)v == e.iEventType)) e.iEventType = (int)v;
            ImGui::EndCombo();
        }
        ImGui::SliderFloat("Start", &e.fTriggerProgress, 0.f, 1.f);
        ImGui::Checkbox("Range", &e.bIsRange);
        if (e.bIsRange) ImGui::SliderFloat("End", &e.fEndProgress, 0.f, 1.f);

        char buf[128]; strcpy_s(buf, e.strParam.c_str());
        if (ImGui::InputText("Param", buf, sizeof(buf))) e.strParam = buf;
        ImGui::InputInt("IntParam", &e.iIntParam);
        ImGui::DragFloat3("Offset", &e.vOffset.x, 0.1f);

        if (ImGui::Button("Delete Event"))
        {
            track.Events.erase(track.Events.begin() + m_iSelEvent);
            m_iSelEvent = -1;
        }
    }
}

CPanel_Animation* CPanel_Animation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CPanel_Animation(pDevice, pContext);
}

void CPanel_Animation::Free() 
{ 
	__super::Free();
 }