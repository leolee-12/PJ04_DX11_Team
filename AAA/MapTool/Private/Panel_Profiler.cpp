#include "Panel_Profiler.h"

#include "Profiler_Manager.h"

CPanel_Profiler::CPanel_Profiler(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel{ pDevice, pContext }
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

	ImGui::Text("Status: %s", PROFILE_ENABLE ? "Enabled" : "Disabled by build");

	Engine::CProfiler_Manager* pEngineProfiler = Engine::CProfiler_Manager::Get();
	if (nullptr == pEngineProfiler)
	{
		ImGui::TextDisabled("Engine Profiler: Unavailable");
		End_Panel();
		return;
	}

	{
		const Engine::PROFILER_FRAME_SNAPSHOT& Snapshot = pEngineProfiler->Get_Snapshot();
		const auto CpuMs = [&Snapshot](Engine::EPROFILE_CPU_SECTION eSection) -> double { return Snapshot.CpuMs[ETOUI(eSection)]; };
		const auto Counter = [&Snapshot](Engine::EPROFILE_COUNTER eCounter) -> _uint { return Snapshot.Counters[ETOUI(eCounter)]; };

		ImGui::Text("Engine Frame: %llu", static_cast<unsigned long long>(Snapshot.iFrameIndex));
		ImGui::Text("FPS avg %.1f / now %.1f", Snapshot.fAvgFPS, Snapshot.fFPS);
		ImGui::Text("Frame %.2f ms / dt %.2f ms", Snapshot.fFrameMs, Snapshot.fDeltaTime * 1000.f);

		ImGui::Separator();
		ImGui::Text("Engine CPU:");
		ImGui::Text("  frame %.2f / update %.2f / render %.2f / present %.2f ms",
			CpuMs(Engine::EPROFILE_CPU_SECTION::FRAME),
			CpuMs(Engine::EPROFILE_CPU_SECTION::UPDATE),
			CpuMs(Engine::EPROFILE_CPU_SECTION::RENDER_TOTAL),
			CpuMs(Engine::EPROFILE_CPU_SECTION::PRESENT));

		ImGui::Separator();
		ImGui::Text("Engine Submitted:");
		ImGui::Text("  total %u / priority %u / shadow %u / nonblend %u",
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_TOTAL),
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_PRIORITY),
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_SHADOW),
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_NONBLEND));
		ImGui::Text("  decal %u / nonlight %u / blend %u / ui %u",
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_DECAL),
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_NONLIGHT),
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_BLEND),
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_UI));

		ImGui::Separator();
		ImGui::Text("Engine Frustum:");
		ImGui::Text("  tested %u / visible %u / culled %u",
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_TESTED),
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_VISIBLE),
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_CULLED));

		ImGui::Text("  fail-open view %u / bounds %u",
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW),
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_BOUNDS));

		ImGui::Separator();
		ImGui::Text("Engine Distance:");
		ImGui::Text("  tested %u / visible %u / culled %u",
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_TESTED),
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_VISIBLE),
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_CULLED));
		ImGui::Text("  fail-open camera %u / bounds %u / distance %u",
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_CAMERA),
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_BOUNDS),
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_DISTANCE));

		ImGui::Separator();
		ImGui::Text("Engine Env Visible:");
		ImGui::Text("  main %u / shadow %u",
			Counter(Engine::EPROFILE_COUNTER::ENV_VISIBLE_MAIN),
			Counter(Engine::EPROFILE_COUNTER::ENV_VISIBLE_SHADOW));

		ImGui::Separator();
		ImGui::Text("Engine Env Instancing:");
		ImGui::Text("  active batches %u / accepted submissions %u",
			Counter(Engine::EPROFILE_COUNTER::ENV_INSTANCE_BATCH_SUBMITTED),
			Counter(Engine::EPROFILE_COUNTER::ENV_INSTANCE_SUBMITTED));
		ImGui::Text("  instanced path %u / batch fallback %u",
			Counter(Engine::EPROFILE_COUNTER::ENV_INSTANCE_RENDERED),
			Counter(Engine::EPROFILE_COUNTER::ENV_INSTANCE_FALLBACK));

		const _uint iTextureRequests = Counter(Engine::EPROFILE_COUNTER::TEXTUREHUB_REQUEST);
		const _uint iTextureDeduped = Counter(Engine::EPROFILE_COUNTER::TEXTUREHUB_DEDUPED);
		const _uint iTextureFirstLoad = Counter(Engine::EPROFILE_COUNTER::TEXTUREHUB_FIRST_LOAD_REQUEST);

		ImGui::Separator();
		ImGui::Text("Engine TextureHub LoadOrGet (Frame):");
		ImGui::Text("  cached SRV %u", Snapshot.iTextureHubCached);
		ImGui::Text("  requests %u / deduped %u / first-load %u / failed %u",
			iTextureRequests,
			iTextureDeduped,
			iTextureFirstLoad,
			Counter(Engine::EPROFILE_COUNTER::TEXTUREHUB_FAILED));

		if (0 == iTextureRequests)
		{
			ImGui::Text("  dedup rate N/A");
		}
		else
		{
			const _float fDedupRate = static_cast<_float>(iTextureDeduped) / static_cast<_float>(iTextureRequests) * 100.f;
			ImGui::Text("  dedup rate %.1f%%", fDedupRate);
		}
	}

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