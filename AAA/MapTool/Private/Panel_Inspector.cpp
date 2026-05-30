#include "Panel_Inspector.h"
#include "imgui.h"

using namespace AnimUITool;

CPanel_Inspector::CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Inspector");
}

void CPanel_Inspector::Render()
{
    ImGui::Begin(m_szName);
    ImGui::Text("Inspector");
    ImGui::End();
}

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Inspector(pDevice, pContext);
}

void CPanel_Inspector::Free() 
{
    __super::Free();
}