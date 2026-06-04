#include "Panel_Browser.h"
#include "Panel_Manager.h"
#include "imgui.h"

using namespace AnimUITool;

static std::string ToLower(std::string s) { for (auto& c : s) c = (char)::tolower(c); return s; }

CPanel_Browser::CPanel_Browser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Browser");
}

HRESULT CPanel_Browser::Initialize(CPanel_Manager* pPanelManager)
{
    if (FAILED(__super::Initialize(pPanelManager)))
        return E_FAIL;

    std::error_code ec;
    m_RootPath = fs::absolute("../../Resources", ec);    
    if (fs::exists(m_RootPath, ec))
        m_RootPath = fs::canonical(m_RootPath, ec);
    m_CurrentPath = m_RootPath;
    m_bNeedRefresh = true;
    return S_OK;
}

void CPanel_Browser::Refresh()
{
    m_Directories.clear();
    m_Files.clear();
    std::error_code ec;
    if (fs::exists(m_CurrentPath, ec))
    {
        for (const auto& entry : fs::directory_iterator(m_CurrentPath, ec))
        {
            if (entry.is_directory(ec)) m_Directories.push_back(entry);
            else                        m_Files.push_back(entry);
        }
        auto byName = [](const fs::directory_entry& a, const fs::directory_entry& b)
            { return a.path().filename().wstring() < b.path().filename().wstring(); };
        std::sort(m_Directories.begin(), m_Directories.end(), byName);
        std::sort(m_Files.begin(), m_Files.end(), byName);
    }
    m_bNeedRefresh = false;
}

void CPanel_Browser::Render()
{
    ImGui::Begin(m_szName);

    if (m_bNeedRefresh) 
        Refresh();

    Render_Breadcrumb();

    ImGui::SameLine();

    if (ImGui::Button("Refresh")) 
        m_bNeedRefresh = true;

    ImGui::Separator();

    Render_Contents();

    ImGui::End();
}

void CPanel_Browser::Render_Breadcrumb()
{
    if (ImGui::Button("Resources")) { m_CurrentPath = m_RootPath; m_bNeedRefresh = true; }

    std::error_code ec;
    fs::path rel = fs::relative(m_CurrentPath, m_RootPath, ec);
    if (!rel.empty() && rel != ".")
    {
        fs::path acc = m_RootPath;
        for (const auto& part : rel)
        {
            acc /= part;
            ImGui::SameLine(); ImGui::Text(">"); ImGui::SameLine();
            if (ImGui::Button(part.string().c_str())) { m_CurrentPath = acc; m_bNeedRefresh = true; }
        }
    }
    if (m_CurrentPath != m_RootPath)
    {
        ImGui::SameLine();
        if (ImGui::Button("<-")) { m_CurrentPath = m_CurrentPath.parent_path(); m_bNeedRefresh = true; }
    }
}

void CPanel_Browser::Render_Contents()
{
    ImGui::BeginChild("Contents", ImVec2(0, 0), true);

    // 폴더 (더블클릭 진입)
    for (const auto& dir : m_Directories)
    {
        std::string label = "[D] " + dir.path().filename().string();
        if (ImGui::Selectable(label.c_str(), m_SelectedPath == dir.path()))
            m_SelectedPath = dir.path();
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            m_CurrentPath = dir.path(); m_SelectedPath = fs::path(); m_bNeedRefresh = true;
        }
    }

    // 파일 (.ysh 더블클릭 → 로드)
    for (const auto& file : m_Files)
    {
        const char* icon = Get_FileIcon(file.path().extension());
        std::string label = std::string(icon) + " " + file.path().filename().string();
        if (ImGui::Selectable(label.c_str(), m_SelectedPath == file.path()))
            m_SelectedPath = file.path();

        if (ToLower(file.path().extension().string()) == ".ysh")
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                m_pPanel_Manager->Load_Preview(file.path().wstring());
    }

    ImGui::EndChild();
}

const char* CPanel_Browser::Get_FileIcon(const fs::path& ext) const
{
    std::string e = ToLower(ext.string());
    if (e == ".ysh")                                   return "[Y]"; 
    if (e == ".hlsl" || e == ".fx")                    return "[S]";
    if (e == ".dds" || e == ".png" || e == ".jpg" ||
        e == ".jpeg" || e == ".bmp" || e == ".tga")     return "[T]";
    if (e == ".fbx" || e == ".obj" || e == ".bfres")   return "[M]";
    if (e == ".json" || e == ".animclips")              return "[C]";
    return "[?]";
}

CPanel_Browser* CPanel_Browser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Browser(pDevice, pContext);
}

void CPanel_Browser::Free() { __super::Free(); }