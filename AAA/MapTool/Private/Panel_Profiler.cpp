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

		struct CULLING_QUERY_DISPLAY
		{
			_float fTested = {};
			_float fVisible = {};
			_float fCulled = {};
		};

		static Engine::PROFILER_FRAME_SNAPSHOT s_PreviousSnapshot = {};
		static _bool s_bHasPreviousSnapshot = false;
		static CULLING_QUERY_DISPLAY s_FrustumDisplay = {};
		static CULLING_QUERY_DISPLAY s_DistanceDisplay = {};

		if (!s_bHasPreviousSnapshot || s_PreviousSnapshot.iFrameIndex != Snapshot.iFrameIndex)
		{
			const _bool bConsecutiveFrame =
				s_bHasPreviousSnapshot &&
				Snapshot.iFrameIndex == s_PreviousSnapshot.iFrameIndex + 1u;

			const auto AverageCounter = [&](Engine::EPROFILE_COUNTER eCounter) -> _float
				{
					const _uint iCurrent = Counter(eCounter);
					if (!bConsecutiveFrame)
						return static_cast<_float>(iCurrent);

					return (static_cast<_float>(s_PreviousSnapshot.Counters[ETOUI(eCounter)])
						+ static_cast<_float>(iCurrent)) * 0.5f;
				};

			s_FrustumDisplay =
			{
					AverageCounter(Engine::EPROFILE_COUNTER::FRUSTUM_TESTED),
					AverageCounter(Engine::EPROFILE_COUNTER::FRUSTUM_VISIBLE),
					AverageCounter(Engine::EPROFILE_COUNTER::FRUSTUM_CULLED)
			};

			s_DistanceDisplay =
			{
					AverageCounter(Engine::EPROFILE_COUNTER::DISTANCE_TESTED),
					AverageCounter(Engine::EPROFILE_COUNTER::DISTANCE_VISIBLE),
					AverageCounter(Engine::EPROFILE_COUNTER::DISTANCE_CULLED)
			};

			s_PreviousSnapshot = Snapshot;
			s_bHasPreviousSnapshot = 0u != Snapshot.iFrameIndex;
		}

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

		const _uint iSubmittedNamed =
			Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_PRIORITY)
			+ Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_SHADOW)
			+ Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_NONBLEND)
			+ Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_DECAL)
			+ Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_NONLIGHT)
			+ Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_BLEND)
			+ Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_UI);
		const _uint iSubmittedTotal = Counter(Engine::EPROFILE_COUNTER::RENDER_SUBMITTED_TOTAL);
		ImGui::Text("  others(sky/blend_hdr/occlusion/distortion) %u",
			iSubmittedTotal >= iSubmittedNamed ? iSubmittedTotal - iSubmittedNamed : 0u);

		ImGui::Separator();
		ImGui::Text("Engine Frustum Queries (2-frame avg):");
		ImGui::Text("  tested %.1f / visible %.1f / culled %.1f",
			s_FrustumDisplay.fTested,
			s_FrustumDisplay.fVisible,
			s_FrustumDisplay.fCulled);

		ImGui::Text("  fail-open view %u / bounds %u",
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_VIEW),
			Counter(Engine::EPROFILE_COUNTER::FRUSTUM_FAIL_OPEN_INVALID_BOUNDS));

		ImGui::Separator();
		ImGui::Text("Engine Distance Queries (2-frame avg):");
		ImGui::Text("  tested %.1f / visible %.1f / culled %.1f",
			s_DistanceDisplay.fTested,
			s_DistanceDisplay.fVisible,
			s_DistanceDisplay.fCulled);

		ImGui::Text("  fail-open camera %u / bounds %u / distance %u",
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_CAMERA),
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_BOUNDS),
			Counter(Engine::EPROFILE_COUNTER::DISTANCE_FAIL_OPEN_INVALID_DISTANCE));

		ImGui::Separator();
		ImGui::Text("Engine Env Visible (every frame, cached result included):");
		ImGui::Text("  culling %.3f ms", CpuMs(Engine::EPROFILE_CPU_SECTION::ENV_CULLING));
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

		const Engine::TEXTURE_HUB_STATS& TextureHub = Snapshot.TextureHub;
		const _uint iUniqueSRVCount = TextureHub.iMaterialRequestCount >= TextureHub.iCacheReuseCount
			? TextureHub.iMaterialRequestCount - TextureHub.iCacheReuseCount
			: 0u;

		ImGui::Separator();
		ImGui::Text("Engine TextureHub MaterialEx (Session):");
		ImGui::Text("  requests %u / unique SRVs %u", TextureHub.iMaterialRequestCount, iUniqueSRVCount);
		ImGui::Text("  hub cached SRVs %u", TextureHub.iCachedSRVCount);
		ImGui::Text("  duplicate loads prevented %u", TextureHub.iCacheReuseCount);

		if (0 == TextureHub.iMaterialRequestCount)
			ImGui::Text("  prevention rate N/A");
		else
			ImGui::Text("  prevention rate %.1f%%",
				static_cast<_float>(TextureHub.iCacheReuseCount) /
				static_cast<_float>(TextureHub.iMaterialRequestCount) * 100.f);

		ImGui::Text("  SRV load failures %u", TextureHub.iLoadFailCount);
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