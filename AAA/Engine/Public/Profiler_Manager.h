#pragma once

#include "Base.h"
#include "Profiler_Defines.h"

NS_BEGIN(Engine)

class ENGINE_DLL CProfiler_Manager final : public CBase
{
public:
	struct DESC
	{
		HWND hWnd = nullptr;
	};

private:
	explicit CProfiler_Manager(const DESC& Desc);
	virtual ~CProfiler_Manager() = default;

public:
	static CProfiler_Manager* Create(const DESC& Desc);
	static CProfiler_Manager* Get();

	void BeginFrame(_float fTimeDelta, _uint64 iFrameIndex);
	void EndFrame();

	_bool BeginCpuSection(EPROFILE_CPU_SECTION eSection);
	void EndCpuSection(EPROFILE_CPU_SECTION eSection);

	void AddCounter(EPROFILE_COUNTER eCounter, _uint iValue = 1);

	void Set_TextureHubStats(const TEXTURE_HUB_STATS& Stats);
	const PROFILER_FRAME_SNAPSHOT& Get_Snapshot() const;

private:
	HRESULT Initialize();
	void Update_WindowTitle(double dNowSec);

private:
	HWND m_hWnd = nullptr;
	_bool m_bFrameOpen = false;

	_uint64 m_iFrameIndex = {};
	double m_dFrameStartSec = {};
	double m_dLastFrameEndSec = {};
	double m_dLastTitleUpdateSec = {};
	double m_TitleCpuMsAccum[ETOUI(EPROFILE_CPU_SECTION::END)] = {};
	_uint m_iTitleFrameCount = {};
	double m_CpuStartSec[ETOUI(EPROFILE_CPU_SECTION::END)] = {};
	_bool m_bCpuSectionRunning[ETOUI(EPROFILE_CPU_SECTION::END)] = {};

	PROFILER_FRAME_SNAPSHOT m_WorkingSnapshot = {};
	PROFILER_FRAME_SNAPSHOT m_Snapshot = {};
	TEXTURE_HUB_STATS m_PrevTextureHubStats = {};

private:
	virtual void Free() override;
};

class ENGINE_DLL CScopedCpuProfile final
{
public:
	explicit CScopedCpuProfile(EPROFILE_CPU_SECTION eSection);
	~CScopedCpuProfile();

private:
	EPROFILE_CPU_SECTION m_eSection = EPROFILE_CPU_SECTION::END;
	_bool m_bActive = false;
};

#define PROFILE_CONCAT_IMPL(A, B) A##B
#define PROFILE_CONCAT(A, B) PROFILE_CONCAT_IMPL(A, B)

#if PROFILE_ENABLE
#define PROFILE_FRAME_BEGIN(DT, IDX) do { if (auto* p = Engine::CProfiler_Manager::Get()) p->BeginFrame(DT, IDX); } while (0)
#define PROFILE_FRAME_END() do { if (auto* p = Engine::CProfiler_Manager::Get()) p->EndFrame(); } while (0)
#define PROFILE_TEXTURE_HUB_STATS(STATS) do { if (auto* p = Engine::CProfiler_Manager::Get()) p->Set_TextureHubStats(STATS); } while (0)
#define PROFILE_CPU_SCOPE(SECTION) Engine::CScopedCpuProfile PROFILE_CONCAT(_cpuProfile_, __LINE__)(SECTION)
#define PROFILE_COUNTER_ADD(ID, V) do { if (auto* p = Engine::CProfiler_Manager::Get()) p->AddCounter(ID, V); } while (0)
#else
#define PROFILE_FRAME_BEGIN(DT, IDX)
#define PROFILE_FRAME_END()
#define PROFILE_TEXTURE_HUB_STATS(STATS)
#define PROFILE_CPU_SCOPE(SECTION)
#define PROFILE_COUNTER_ADD(ID, V)
#endif

NS_END