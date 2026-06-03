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
	const Client::MAP_STAGE_PROFILE& Frame = pProfiler->Get_Frame();

	_bool bEnabled = pProfiler->Is_Enabled();
	if (ImGui::Checkbox("Enabled", &bEnabled))
		pProfiler->Set_Enabled(bEnabled);

	ImGui::SameLine();

	_bool bCsv = pProfiler->Is_CsvEnabled();
	if (ImGui::Checkbox("CSV", &bCsv))
		pProfiler->Set_CsvEnabled(bCsv);

	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		pProfiler->Reset();

	ImGui::Separator();
	ImGui::Text("Stage: %s", pProfiler->Has_Stage() ? "Connected" : "None");
	ImGui::Text("Frame: %u", Frame.iFrameIndex);
	ImGui::Text("Sections: total %u / visible %u / culled %u",
		Frame.iTotalSections,
		Frame.iVisibleSections,
		Frame.iCulledSections);
	ImGui::Text("Submitted: nonblend %u / blend %u / shadow %u",
		Frame.iSubmittedNonBlend,
		Frame.iSubmittedBlend,
		Frame.iSubmittedShadow);

	ImGui::Separator();
	ImGui::Text("Stage LateUpdate: %.3f ms", Frame.dStageLateUpdateCpuMs);
	ImGui::Text("Culling: %.3f ms", Frame.dCullingCpuMs);
	ImGui::Text("Section Render: %.3f ms", Frame.dSectionRenderCpuMs);
	ImGui::Text("Estimated Draw Calls: %u", Frame.iEstimatedDrawCalls);

	ImGui::Separator();
	ImGui::Text("Type Counts");
	ImGui::Text("Default: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::DEFAULT)]);
	ImGui::Text("Building: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::BUILDING)]);
	ImGui::Text("Ground: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::GROUND)]);
	ImGui::Text("Rock: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::ROCK)]);
	ImGui::Text("Transparent: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::TRANSPARENT)]);
	ImGui::Text("Water: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::WATER)]);
	ImGui::Text("Effect: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::EFFECT)]);
	ImGui::Text("Unknown: %u", Frame.iSectionTypeCount[static_cast<_uint>(Client::MAP_SECTION_TYPE::UNKNOWN)]);

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
