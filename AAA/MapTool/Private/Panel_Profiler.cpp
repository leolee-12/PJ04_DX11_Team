#include "Panel_Profiler.h"

#ifdef _DEBUG

#include "MapToolProfiler.h"

CPanel_Profiler::CPanel_Profiler(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel { pDevice, pContext }
{
	strcpy_s(m_szName, "Profiler");
}

void CPanel_Profiler::Render()
{
    if (!Begin_Panel())
    {
        End_Panel();
        return;
    }

    CMapToolProfiler* pProfiler = CMapToolProfiler::GetInstance();
    const MAPTOOL_PROFILE_FRAME& Frame = pProfiler->Get_Frame();

    _bool bEnabled = pProfiler->Is_Enabled();
    if (ImGui::Checkbox("Enabled", &bEnabled))
        pProfiler->Set_Enabled(bEnabled);

    ImGui::SameLine();
    if (ImGui::Button("Reset"))
        pProfiler->Reset();

    ImGui::Separator();
    ImGui::Text("Stage: %s", pProfiler->Has_Stage() ? "Connected" : "None");
    ImGui::Text("Frame: %u", Frame.iFrameIndex);

    ImGui::Separator();
    ImGui::Text("Map Sections: %u", Frame.iMapSectionTotal);
    ImGui::Text("Map Main: candidate %u / visible %u / culled %u / submitted %u",
        Frame.MapMain.iCandidate,
        Frame.MapMain.iVisible,
        Frame.MapMain.iCulled,
        Frame.MapMain.iSubmitted);
    ImGui::Text("Map Shadow: candidate %u / visible %u / culled %u / submitted %u",
        Frame.MapShadow.iCandidate,
        Frame.MapShadow.iVisible,
        Frame.MapShadow.iCulled,
        Frame.MapShadow.iSubmitted);

    ImGui::Separator();
    ImGui::Text("Env Objects: %u", Frame.iEnvObjectTotal);
    ImGui::Text("Env Main: candidate %u / visible %u / culled %u / submitted %u",
        Frame.EnvMain.iCandidate,
        Frame.EnvMain.iVisible,
        Frame.EnvMain.iCulled,
        Frame.EnvMain.iSubmitted);
    ImGui::Text("Env Shadow: candidate %u / visible %u / culled %u / submitted %u",
        Frame.EnvShadow.iCandidate,
        Frame.EnvShadow.iVisible,
        Frame.EnvShadow.iCulled,
        Frame.EnvShadow.iSubmitted);

    ImGui::Separator();
    ImGui::Text("Map Culling: %.3f ms", Frame.dMapCullingCpuMs);

    ImGui::Separator();
    ImGui::Text("Texture Hub: unique %u / hit %u / miss %u / raw fail %u",
        Frame.iTextureHubUnique,
        Frame.iTextureHubHit,
        Frame.iTextureHubMiss,
        Frame.iTextureHubRawFail);

    End_Panel();
}

CPanel_Profiler* CPanel_Profiler::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CPanel_Profiler(pDevice, pContext);
}

void CPanel_Profiler::Free()
{
	__super::Free();
}

#endif
