#include "ImGui_Manager.h"
#include "Level_Edit.h"
#include "GameInstance.h"
#include "GameInstance_Proxy.h"

using namespace Editor;
using namespace Engine;
using namespace Client;

static const ImGuiWindowFlags PANEL_FLAGS =
ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

IMPLEMENT_SINGLETON(CImGui_Manager);

CImGui_Manager::CImGui_Manager()
    : m_pGameInstance_Proxy(CGameInstance::GetProxy())
{
}

CImGui_Manager::~CImGui_Manager()
{
}

void CImGui_Manager::Viewport_DisableBlend(const ImDrawList*, const ImDrawCmd* cmd)
{
    VIEWPORT_DRAW* p = (VIEWPORT_DRAW*)cmd->UserCallbackData;
    if (p && p->pContext)
        p->pContext->OMSetBlendState(p->pBlend, nullptr, 0xffffffff);
}

HRESULT CImGui_Manager::ImGui_Initialize(ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, CLevel_Edit*
    pLevelEdit, ID3D11ShaderResourceView** ppSRV, ID3D11ShaderResourceView** ppSRV2)
{
    if (!pLevelEdit || !ppDevice || !ppContext) { MSG_BOX("ImGui_Initialize Failed"); return E_FAIL; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplWin32_Init(g_hWndEditor);
    ImGui_ImplDX11_Init(*ppDevice, *ppContext);

    m_pLevel_Edit = pLevelEdit; Safe_AddRef(m_pLevel_Edit);
    m_pSRV = *ppSRV;            Safe_AddRef(m_pSRV);
    m_pSRV2 = *ppSRV2;          Safe_AddRef(m_pSRV2);
    m_pContext = *ppContext;    Safe_AddRef(m_pContext);

    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = FALSE;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED((*ppDevice)->CreateBlendState(&bd, &m_pOpaqueBlend)))
        return E_FAIL;

    m_ViewportDraw = { m_pContext, m_pOpaqueBlend };
    m_eGizmoOp = ImGuizmo::TRANSLATE;
    return S_OK;
}

void CImGui_Manager::ImGui_Render()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 P = vp->Pos, S = vp->Size;
    float topH = 90.f, leftW = S.x * 0.16f, rightW = S.x * 0.24f;
    float cy = P.y + topH, ch = S.y - topH;

    ImGui::SetNextWindowPos(P, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(S.x, topH), ImGuiCond_Always); Draw_Toolbar();

    ImGui::SetNextWindowPos(ImVec2(P.x, cy), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(leftW, ch), ImGuiCond_Always); Draw_List();

    ImGui::SetNextWindowPos(ImVec2(P.x + leftW, cy), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(S.x - leftW - rightW, ch), ImGuiCond_Always); Draw_Viewport();

    ImGui::SetNextWindowPos(ImVec2(P.x + S.x - rightW, cy), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(rightW, ch), ImGuiCond_Always); Draw_Inspector();

    Draw_Preview();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void CImGui_Manager::Draw_Toolbar()
{
    ImGui::Begin("Toolbar", nullptr, PANEL_FLAGS);
    auto& solver = m_pLevel_Edit->Get_Solver();

    ImGui::SetNextItemWidth(420);
    ImGui::InputText("##doc", m_szDocPath, sizeof(m_szDocPath));
    ImGui::SameLine();
    if (ImGui::Button("Load Doc")) {
        wstring w(m_szDocPath, m_szDocPath + strlen(m_szDocPath));
        m_pLevel_Edit->Load_CameraDoc(w);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Doc")) {
        wstring w(m_szDocPath, m_szDocPath + strlen(m_szDocPath));
        m_pLevel_Edit->Save_CameraDoc(w);
    }
    ImGui::SameLine(); if (ImGui::Button("L0"))    strcpy_s(m_szDocPath, "../../Tools/Level0_Stage1_Step01_cam.json");
    ImGui::SameLine(); if (ImGui::Button("L1"))    strcpy_s(m_szDocPath, "../../Tools/Level1_Stage1_Step01_cam.json");
    ImGui::SameLine(); if (ImGui::Button("Arena")) strcpy_s(m_szDocPath, "../../Tools/Arena_Step01_cam.json");

    if (ImGui::RadioButton("Translate", m_eGizmoOp == ImGuizmo::TRANSLATE)) m_eGizmoOp = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", m_eGizmoOp == ImGuizmo::ROTATE)) m_eGizmoOp = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", m_eGizmoOp == ImGuizmo::SCALE)) m_eGizmoOp = ImGuizmo::SCALE; ImGui::SameLine();
    ImGui::Dummy(ImVec2(16, 0)); ImGui::SameLine();

    bool prev = m_pLevel_Edit->Is_Preview();
    if (ImGui::Checkbox("Preview Camera", &prev)) m_pLevel_Edit->Set_Preview(prev);
    ImGui::SameLine();
    if (ImGui::Button("Add Area"))
    {
        CAM_AREA A; A.center = m_pLevel_Edit->Kirby(); A.size = _float3(20.f, 10.f, 20.f);
        solver.Areas().push_back(A); m_pLevel_Edit->Select_Area((_int)solver.Areas().size() - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Rail"))
    {
        CAM_RAIL R; R.uid = (_uint)GetTickCount64(); _float3 k = m_pLevel_Edit->Kirby();
        R.nodes.push_back(_float3(k.x - 5.f, k.y, k.z)); R.nodes.push_back(_float3(k.x + 5.f, k.y, k.z));
        solver.Rails().push_back(R);
    }

    _uint nP = m_pLevel_Edit->Get_MapPreviewPresetCount();
    if (nP > 0)
    {
        static int s_p = 0; if (s_p >= (int)nP) s_p = 0;
        ImGui::SetNextItemWidth(220);
        if (ImGui::BeginCombo("##map", m_pLevel_Edit->Get_MapPreviewPresetLabel(s_p)))
        {
            for (_uint i = 0; i < nP; ++i)
                if (ImGui::Selectable(m_pLevel_Edit->Get_MapPreviewPresetLabel(i), (int)i == s_p)) s_p = i;
            ImGui::EndCombo();
        }
        // both
        ImGui::SameLine(); if (ImGui::Button("Load Map"))   m_pLevel_Edit->Load_MapPreview((_uint)s_p);
        ImGui::SameLine(); if (ImGui::Button("Clear Map"))  m_pLevel_Edit->Clear_MapPreview();
        // stage only
        ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();
        if (ImGui::Button("Load Stage"))  m_pLevel_Edit->Load_MapPreviewStage((_uint)s_p);
        ImGui::SameLine(); if (ImGui::Button("Clear Stage")) m_pLevel_Edit->Clear_MapPreviewStage();
        // env only
        ImGui::SameLine(); ImGui::TextDisabled("|"); ImGui::SameLine();
        if (ImGui::Button("Load Env"))    m_pLevel_Edit->Load_MapPreviewEnv((_uint)s_p);
        ImGui::SameLine(); if (ImGui::Button("Clear Env"))   m_pLevel_Edit->Clear_MapPreviewEnv();
    }
    // status line
    {
        const _wstring& st = m_pLevel_Edit->Get_MapPreviewStatus();
        string s = WstrToStr(st);
        ImGui::SameLine(); ImGui::TextDisabled("%s", s.c_str());
    }
    ImGui::SameLine(); ImGui::TextDisabled("RMB: fly / WASD / Wheel / F2: debugdraw");

    ImGui::End();
}

void CImGui_Manager::Draw_List()
{
    ImGui::Begin("Scene", nullptr, PANEL_FLAGS);
    auto& solver = m_pLevel_Edit->Get_Solver();
    auto  sel = m_pLevel_Edit->Get_SelType();

    if (ImGui::Selectable("[Kirby Proxy]", sel == CLevel_Edit::SEL::KIRBY)) m_pLevel_Edit->Select_Kirby();

    ImGui::SeparatorText("Areas");
    auto& areas = solver.Areas();
    for (int i = 0; i < (int)areas.size(); ++i)
    {
        char b[64];
        sprintf_s(b, "Area %d  p%d %s##a%d", i, areas[i].priority,
            areas[i].mode == CAM_MODE::FIXED_GAZING ? "FG" : "N", i);
        bool s = (sel == CLevel_Edit::SEL::AREA && m_pLevel_Edit->Get_SelArea() == i);
        if (ImGui::Selectable(b, s)) m_pLevel_Edit->Select_Area(i);
    }

    ImGui::SeparatorText("Rails");
    auto& rails = solver.Rails();
    for (int r = 0; r < (int)rails.size(); ++r)
    {
        char b[64]; sprintf_s(b, "Rail %u (%zu)##r%d", rails[r].uid, rails[r].nodes.size(), r);
        if (ImGui::TreeNode(b))
        {
            for (int n = 0; n < (int)rails[r].nodes.size(); ++n)
            {
                char nb[32]; sprintf_s(nb, "node %d##%d_%d", n, r, n);
                bool s = (sel == CLevel_Edit::SEL::NODE && m_pLevel_Edit->Get_SelRail() == r &&
                    m_pLevel_Edit->Get_SelNode() == n);
                if (ImGui::Selectable(nb, s)) m_pLevel_Edit->Select_Node(r, n);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void CImGui_Manager::Draw_OffsetEditor(const _char* label, CAM_OFFSET& o)
{
    if (!ImGui::TreeNode(label)) return;
    ImGui::DragFloat3("targetOffs", &o.targetOffs.x, 0.05f);
    ImGui::DragFloat3("snapCenter", &o.snapCenter.x, 0.1f);
    ImGui::DragFloat3("snapRate", &o.snapRate.x, 0.01f, 0.f, 1.f);
    ImGui::DragFloat4("snapRot", &o.snapRot.x, 0.01f);
    ImGui::DragFloat3("eyeSnapCenter", &o.eyeSnapCenter.x, 0.1f);
    ImGui::DragFloat3("eyeSnapRate", &o.eyeSnapRate.x, 0.01f, 0.f, 1.f);
    ImGui::DragFloat4("eyeSnapRot", &o.eyeSnapRot.x, 0.01f);
    ImGui::DragFloat("dist", &o.dist, 0.1f);
    ImGui::DragFloat("tilt", &o.tilt, 0.1f);
    ImGui::DragFloat("rotY", &o.rotY, 0.1f);
    ImGui::DragFloat("roll", &o.roll, 0.1f);
    ImGui::DragFloat("fovY", &o.fovY, 0.1f);
    ImGui::TreePop();
}

void CImGui_Manager::Draw_AreaInspector(CAM_AREA& A, _int idx)
{
    ImGui::Text("Area %d", idx);
    const char* modes[] = { "Normal", "FixedGazing" };
    int m = (A.mode == CAM_MODE::FIXED_GAZING) ? 1 : 0;
    if (ImGui::Combo("mode", &m, modes, 2)) A.mode = m ? CAM_MODE::FIXED_GAZING : CAM_MODE::NORMAL;

    ImGui::DragFloat3("center", &A.center.x, 0.1f);
    ImGui::DragFloat3("size", &A.size.x, 0.1f, 0.01f, 100000.f);
    ImGui::DragFloat4("rot (quat)", &A.rot.x, 0.01f);
    ImGui::InputInt("priority", &A.priority);
    ImGui::DragFloat("erpIn", &A.erpIn, 1.f);
    ImGui::DragFloat("erpOut", &A.erpOut, 1.f);

    ImGui::Checkbox("useRail", &A.useRail);
    {
        char cur[32]; sprintf_s(cur, "%u", A.railUid);
        if (ImGui::BeginCombo("railUid", cur))
        {
            for (auto& R : m_pLevel_Edit->Get_Solver().Rails())
            {
                char b[32]; sprintf_s(b, "%u", R.uid);
                if (ImGui::Selectable(b, R.uid == A.railUid)) A.railUid = R.uid;
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Checkbox("eyeSnap", &A.eyeSnap); ImGui::SameLine();
    ImGui::Checkbox("targetSnap", &A.targetSnap);
    ImGui::Checkbox("usePanLimit", &A.usePanLimit);
    ImGui::DragFloat("panCenter", &A.panCenter, 0.5f);
    ImGui::DragFloat("panRange", &A.panRange, 0.5f);

    ImGui::Separator();
    Draw_OffsetEditor("Base offset", A.base);
    Draw_OffsetEditor("End offset", A.end);
    ImGui::Separator();

    if (ImGui::Button("Duplicate"))
    {
        auto& areas = m_pLevel_Edit->Get_Solver().Areas();
        areas.push_back(A);
        m_pLevel_Edit->Select_Area((_int)areas.size() - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
    {
        auto& areas = m_pLevel_Edit->Get_Solver().Areas();
        areas.erase(areas.begin() + idx);
        m_pLevel_Edit->Select_None();
    }
}

void CImGui_Manager::Draw_NodeInspector()
{
    auto& rails = m_pLevel_Edit->Get_Solver().Rails();
    int r = m_pLevel_Edit->Get_SelRail(), n = m_pLevel_Edit->Get_SelNode();
    if (r < 0 || r >= (int)rails.size()) return;
    auto& R = rails[r];

    ImGui::Text("Rail uid %u", R.uid);
    ImGui::Checkbox("close", &R.close);
    if (n < 0 || n >= (int)R.nodes.size()) return;

    ImGui::DragFloat3("node pos", &R.nodes[n].x, 0.1f);
    if (ImGui::Button("Add node after"))
    {
        R.nodes.insert(R.nodes.begin() + n + 1, R.nodes[n]);
        m_pLevel_Edit->Select_Node(r, n + 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove node"))
    {
        R.nodes.erase(R.nodes.begin() + n);
        m_pLevel_Edit->Select_None();
    }
    ImGui::Separator();
    if (ImGui::Button("Delete Rail"))
    {
        rails.erase(rails.begin() + r);
        m_pLevel_Edit->Select_None();
    }
}

void CImGui_Manager::Draw_KirbyInspector()
{
    _float3& k = m_pLevel_Edit->Kirby();
    ImGui::DragFloat3("kirby pos", &k.x, 0.1f);

    auto& solver = m_pLevel_Edit->Get_Solver();
    CAM_POSE p = solver.Solve(XMLoadFloat3(&k));
    ImGui::Separator();
    ImGui::Text("current area : %d", solver.Cur_AreaIndex());
    ImGui::Text("eye  %.2f %.2f %.2f", p.eye.x, p.eye.y, p.eye.z);
    ImGui::Text("fwd  %.2f %.2f %.2f", p.fwd.x, p.fwd.y, p.fwd.z);
    ImGui::Text("fov  %.1f", p.fov);
}

void CImGui_Manager::Draw_Inspector()
{
    ImGui::Begin("Inspector", nullptr, PANEL_FLAGS);
    auto  sel = m_pLevel_Edit->Get_SelType();
    auto& areas = m_pLevel_Edit->Get_Solver().Areas();

    if (sel == CLevel_Edit::SEL::AREA)
    {
        int i = m_pLevel_Edit->Get_SelArea();
        if (i >= 0 && i < (int)areas.size()) Draw_AreaInspector(areas[i], i);
    }
    else if (sel == CLevel_Edit::SEL::NODE) Draw_NodeInspector();
    else if (sel == CLevel_Edit::SEL::KIRBY) Draw_KirbyInspector();
    else ImGui::TextWrapped("Select an area / rail node / kirby proxy from the Scene panel.");

    ImGui::End();
}

void CImGui_Manager::Draw_Viewport()
{
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float ta = 1600.f / 900.f, aa = avail.x / avail.y; ImVec2 sz;
    if (aa > ta) { sz.y = avail.y; sz.x = avail.y * ta; }
    else { sz.x = avail.x; sz.y = avail.x / ta; }
    ImVec2 off((avail.x - sz.x) * 0.5f, (avail.y - sz.y) * 0.5f);
    ImVec2 cur = ImGui::GetCursorPos(); ImGui::SetCursorPos(ImVec2(cur.x + off.x, cur.y + off.y));
    ImVec2 vPos = ImGui::GetCursorScreenPos();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCallback(Viewport_DisableBlend, &m_ViewportDraw);
    ImGui::Image((ImTextureID)m_pSRV, sz);
    dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
    const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
    if (!pView || !pProj) { ImGui::End(); return; }

    auto W2S = [&](const _float3& w) -> ImVec2 {
        XMVECTOR clip = XMVector4Transform(XMVectorSet(w.x, w.y, w.z, 1.f), XMLoadFloat4x4(pView) *
            XMLoadFloat4x4(pProj));
        float cw = XMVectorGetW(clip); if (cw < 1e-5f) return ImVec2(-9999.f, -9999.f);
        float x = XMVectorGetX(clip) / cw, y = XMVectorGetY(clip) / cw;
        return ImVec2(vPos.x + (x + 1.f) * 0.5f * sz.x, vPos.y + (1.f - y) * 0.5f * sz.y);
        };

    bool bRight = ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);
    m_pLevel_Edit->Set_CameraActive(bRight);

    auto& solver = m_pLevel_Edit->Get_Solver();
    auto& areas = solver.Areas();
    auto& rails = solver.Rails();
    _float3 kirby = m_pLevel_Edit->Kirby();
    CAM_POSE pose = solver.Solve(XMLoadFloat3(&kirby));
    int curArea = solver.Cur_AreaIndex();
    auto sel = m_pLevel_Edit->Get_SelType();

    auto DrawOBB = [&](const CAM_AREA& A, ImU32 col) {
        _vector c = XMLoadFloat3(&A.center), q = XMLoadFloat4(&A.rot);
        _float3 h; XMStoreFloat3(&h, XMVectorScale(XMLoadFloat3(&A.size), 0.5f));
        const float sx[8] = { -1,1,1,-1,-1,1,1,-1 }, sy[8] = { -1,-1,1,1,-1,-1,1,1 }, sz3[8] = { -1,-1,-1,-1,1,1,1,1
        };
        ImVec2 p[8];
        for (int k = 0; k < 8; ++k) {
            _vector l = XMVectorSet(sx[k] * h.x, sy[k] * h.y, sz3[k] * h.z, 0.f);
            _float3 wf; XMStoreFloat3(&wf, XMVectorAdd(c, XMVector3Rotate(l, q))); p[k] = W2S(wf);
        }
        const int e[12][2] = { {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7} };
        for (auto& ed : e) if (p[ed[0]].x > -9000.f && p[ed[1]].x > -9000.f) dl->AddLine(p[ed[0]], p[ed[1]], col,
            1.5f);
        };

    for (int i = 0; i < (int)areas.size(); ++i) {
        ImU32 col = (sel == CLevel_Edit::SEL::AREA && m_pLevel_Edit->Get_SelArea() == i) ? IM_COL32(255, 220, 60, 255)
            : (i == curArea) ? IM_COL32(80, 220, 120, 255) : IM_COL32(150, 150, 150, 170);
        DrawOBB(areas[i], col);
    }
    for (int r = 0; r < (int)rails.size(); ++r) {
        auto& R = rails[r];
        for (int n = 0; n < (int)R.nodes.size(); ++n) {
            ImVec2 a = W2S(R.nodes[n]); if (a.x < -9000.f) continue;
            bool s = (sel == CLevel_Edit::SEL::NODE && m_pLevel_Edit->Get_SelRail() == r &&
                m_pLevel_Edit->Get_SelNode() == n);
            dl->AddCircleFilled(a, s ? 6.f : 4.f, s ? IM_COL32(255, 230, 80, 255) : IM_COL32(80, 160, 255, 255));
            if (n + 1 < (int)R.nodes.size()) {
                ImVec2 b = W2S(R.nodes[n + 1]); if (b.x > -9000.f) dl->AddLine(a, b,
                    IM_COL32(80, 160, 255, 200), 2.f);
            }
        }
    }
    {
        ImVec2 kp = W2S(kirby);
        if (kp.x > -9000.f) {
            dl->AddCircleFilled(kp, 5.f, IM_COL32(255, 90, 90, 255));
            dl->AddLine(ImVec2(kp.x - 8, kp.y), ImVec2(kp.x + 8, kp.y), IM_COL32(255, 255, 255, 255));
            dl->AddLine(ImVec2(kp.x, kp.y - 8), ImVec2(kp.x, kp.y + 8), IM_COL32(255, 255, 255, 255));
        }
    }
    { // solved camera frustum
        _vector E = XMLoadFloat3(&pose.eye), F = XMVector3Normalize(XMLoadFloat3(&pose.fwd)), Uref =
            XMLoadFloat3(&pose.up);
        _vector Rr = XMVector3Cross(Uref, F);
        if (XMVectorGetX(XMVector3LengthSq(Rr)) < 1e-6f) Rr = XMVector3Cross(XMVectorSet(0, 0, 1, 0), F);
        Rr = XMVector3Normalize(Rr); _vector U = XMVector3Normalize(XMVector3Cross(F, Rr));
        const float d = 28.f, asp = 16.f / 9.f;
        float hh = d * tanf(XMConvertToRadians(pose.fov) * 0.5f), hw = hh * asp;
        _vector C = XMVectorAdd(E, XMVectorScale(F, d));
        _float3 fc[4]; ImVec2 pe = W2S(pose.eye), pc[4];
        _vector corn[4] = {
            XMVectorAdd(C, XMVectorAdd(XMVectorScale(Rr,-hw), XMVectorScale(U, hh))),
            XMVectorAdd(C, XMVectorAdd(XMVectorScale(Rr, hw), XMVectorScale(U, hh))),
            XMVectorAdd(C, XMVectorAdd(XMVectorScale(Rr, hw), XMVectorScale(U,-hh))),
            XMVectorAdd(C, XMVectorAdd(XMVectorScale(Rr,-hw), XMVectorScale(U,-hh))) };
        for (int i = 0; i < 4; ++i) { XMStoreFloat3(&fc[i], corn[i]); pc[i] = W2S(fc[i]); }
        ImU32 oc = IM_COL32(255, 170, 40, 255);
        if (pe.x > -9000.f) {
            dl->AddCircleFilled(pe, 4.f, oc); for (int i = 0; i < 4; ++i) if (pc[i].x > -9000.f)
                dl->AddLine(pe, pc[i], oc, 1.5f);
        }
        for (int i = 0; i < 4; ++i) {
            int j = (i + 1) % 4; if (pc[i].x > -9000.f && pc[j].x > -9000.f)
                dl->AddLine(pc[i], pc[j], oc, 1.5f);
        }
    }

    // ---- gizmo ----
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(vPos.x, vPos.y, sz.x, sz.y);

    if (sel == CLevel_Edit::SEL::AREA) {
        int i = m_pLevel_Edit->Get_SelArea();
        if (i >= 0 && i < (int)areas.size()) {
            ImGuizmo::PushID(0);
            CAM_AREA& A = areas[i];
            XMMATRIX W = XMMatrixScalingFromVector(XMVectorMax(XMLoadFloat3(&A.size), XMVectorReplicate(0.01f)))
                * XMMatrixRotationQuaternion(XMVector4Normalize(XMLoadFloat4(&A.rot)))
                * XMMatrixTranslationFromVector(XMLoadFloat3(&A.center));
            _float4x4 m; XMStoreFloat4x4(&m, W);
            ImGuizmo::Manipulate((float*)pView, (float*)pProj, m_eGizmoOp, ImGuizmo::LOCAL, (float*)&m);
            if (ImGuizmo::IsUsing()) {
                XMVECTOR vs, vq, vt;
                if (XMMatrixDecompose(&vs, &vq, &vt, XMLoadFloat4x4(&m))) {
                    XMStoreFloat3(&A.size, XMVectorMax(vs, XMVectorReplicate(0.01f)));
                    XMStoreFloat4(&A.rot, XMQuaternionNormalize(vq));
                    XMStoreFloat3(&A.center, vt);
                }
            }
            ImGuizmo::PopID();
        }
    }
    else if (sel == CLevel_Edit::SEL::NODE) {
        int r = m_pLevel_Edit->Get_SelRail(), n = m_pLevel_Edit->Get_SelNode();
        if (r >= 0 && r < (int)rails.size() && n >= 0 && n < (int)rails[r].nodes.size()) {
            ImGuizmo::PushID(0);
            _float3& nd = rails[r].nodes[n];
            _float4x4 m; XMStoreFloat4x4(&m, XMMatrixTranslationFromVector(XMLoadFloat3(&nd)));
            ImGuizmo::Manipulate((float*)pView, (float*)pProj, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, (float*)&m);
            if (ImGuizmo::IsUsing()) { nd.x = m.m[3][0]; nd.y = m.m[3][1]; nd.z = m.m[3][2]; }
            ImGuizmo::PopID();
        }
    }
    {   // kirby gizmo - always
        ImGuizmo::PushID(1);
        _float3& k = m_pLevel_Edit->Kirby();
        _float4x4 m; XMStoreFloat4x4(&m, XMMatrixTranslationFromVector(XMLoadFloat3(&k)));
        ImGuizmo::Manipulate((float*)pView, (float*)pProj, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, (float*)&m);
        if (ImGuizmo::IsUsing()) { k.x = m.m[3][0]; k.y = m.m[3][1]; k.z = m.m[3][2]; }
        ImGuizmo::PopID();
    }

    ImGui::End();
}

void CImGui_Manager::Draw_Preview()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + vp->Size.y * 0.5f),
        ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(680, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Preview (game cam)", nullptr, ImGuiWindowFlags_NoScrollbar);

    if (!m_pLevel_Edit->Is_Preview())
    {
        ImGui::TextWrapped("Preview OFF.  Turn on 'Preview Camera' in the toolbar.");
        ImGui::End();
        return;
    }
    if (nullptr == m_pSRV2)
    {
        ImGui::TextWrapped("m_pSRV2 is null (preview RTV not created).");
        ImGui::End();
        return;
    }

    ImVec2 a = ImGui::GetContentRegionAvail();
    float ta = 1600.f / 900.f, aa = (a.y > 0.f ? a.x / a.y : ta); ImVec2 sz;
    if (aa > ta) { sz.y = a.y; sz.x = a.y * ta; }
    else { sz.x = a.x; sz.y = a.x / ta; }
    ImVec2 off((a.x - sz.x) * 0.5f, (a.y - sz.y) * 0.5f);
    ImVec2 cur = ImGui::GetCursorPos(); ImGui::SetCursorPos(ImVec2(cur.x + off.x, cur.y + off.y));

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddCallback(Viewport_DisableBlend, &m_ViewportDraw);
    ImGui::Image((ImTextureID)m_pSRV2, sz);
    dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

    ImGui::End();
}

void CImGui_Manager::Free()
{
    Safe_Release(m_pOpaqueBlend);
    Safe_Release(m_pContext);
    Safe_Release(m_pSRV);
    Safe_Release(m_pSRV2);
    Safe_Release(m_pLevel_Edit);

    Safe_Release(m_pGameInstance_Proxy);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}