#include "Panel_Console.h"
#include "imgui.h"

CPanel_Console::CPanel_Console(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Console");
}

void CPanel_Console::Render()
{
    if (!Begin_Panel())
    {
        End_Panel();
        return;
    }

    auto FilterButton = [](const char* szLabel, _bool& bShow, ImVec4 vOnColor)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, bShow ? vOnColor : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            if (ImGui::SmallButton(szLabel))
                bShow = !bShow;
            ImGui::PopStyleColor();
        };

    FilterButton("Info", m_bShowInfo, ImVec4(0.2f, 0.7f, 0.3f, 1.0f)); ImGui::SameLine();
    FilterButton("Warning", m_bShowWarning, ImVec4(0.9f, 0.8f, 0.2f, 1.0f)); ImGui::SameLine();
    FilterButton("Error", m_bShowError, ImVec4(0.9f, 0.2f, 0.2f, 1.0f)); ImGui::SameLine();
    if (ImGui::SmallButton("Clear"))
        Clear_Log();

    ImGui::Separator();
    ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& Entry : Get_LogBuffer())
    {
        if (Entry.eLevel == LOG_LEVEL::INFO && !m_bShowInfo)    continue;
        if (Entry.eLevel == LOG_LEVEL::WARNING && !m_bShowWarning) continue;
        if (Entry.eLevel == LOG_LEVEL::ERROR_ && !m_bShowError)   continue;

        ImVec4 vColor; const char* szPrefix = "";
        switch (Entry.eLevel)
        {
        case LOG_LEVEL::INFO:    vColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); szPrefix = "[Info]    "; break;
        case LOG_LEVEL::WARNING: vColor = ImVec4(1.0f, 0.9f, 0.3f, 1.0f); szPrefix = "[Warning] "; break;
        case LOG_LEVEL::ERROR_:  vColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); szPrefix = "[Error]   "; break;
        default:                 vColor = ImVec4(1, 1, 1, 1);                                       break;
        }
        ImGui::TextColored(vColor, "%s%s", szPrefix, Entry.strMessage.c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    End_Panel();
}

CPanel_Console* CPanel_Console::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Console(pDevice, pContext);
}

void CPanel_Console::Free()
{
    __super::Free();
}
