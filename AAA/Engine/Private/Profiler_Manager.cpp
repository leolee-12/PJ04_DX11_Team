#include "Profiler_Manager.h"

#include <chrono>
#include <cwchar>

NS_BEGIN(Engine)

namespace
{
	CProfiler_Manager* g_pProfiler_Manager = nullptr;
	constexpr double TITLE_UPDATE_INTERVAL = 0.25;

	double NowSec()
	{
		using Clock = std::chrono::steady_clock;
		return std::chrono::duration<double>(Clock::now().time_since_epoch()).count();
	}
}

CProfiler_Manager::CProfiler_Manager(const DESC& Desc)
	: m_hWnd(Desc.hWnd)
{
}

CProfiler_Manager* CProfiler_Manager::Create(const DESC& Desc)
{
	if (nullptr != g_pProfiler_Manager)
		return g_pProfiler_Manager;

	CProfiler_Manager* pInstance = new CProfiler_Manager(Desc);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CProfiler_Manager");
		Safe_Release(pInstance);
		return nullptr;
	}

	g_pProfiler_Manager = pInstance;
	return pInstance;
}

CProfiler_Manager* CProfiler_Manager::Get()
{
	return g_pProfiler_Manager;
}

HRESULT CProfiler_Manager::Initialize()
{
	m_dLastTitleUpdateSec = NowSec();
	return S_OK;
}

void CProfiler_Manager::BeginFrame(_float fTimeDelta, _uint64 iFrameIndex)
{
	m_WorkingSnapshot = {};
	m_WorkingSnapshot.fAvgFPS = m_Snapshot.fAvgFPS;
	m_WorkingSnapshot.iFrameIndex = iFrameIndex;
	m_WorkingSnapshot.fDeltaTime = fTimeDelta;

	m_iFrameIndex = iFrameIndex;
	m_dFrameStartSec = NowSec();
	m_bFrameOpen = true;

	for (_uint i = 0; i < ETOUI(EPROFILE_CPU_SECTION::END); ++i)
	{
		m_CpuStartSec[i] = 0.0;
		m_bCpuSectionRunning[i] = false;
	}
}

void CProfiler_Manager::EndFrame()
{
	if (!m_bFrameOpen)
		return;

	const double dNowSec = NowSec();
	const double dWorkMs = (dNowSec - m_dFrameStartSec) * 1000.0;

	double dFrameIntervalMs = dWorkMs;
	if (0.0 < m_dLastFrameEndSec)
		dFrameIntervalMs = (dNowSec - m_dLastFrameEndSec) * 1000.0;
	m_dLastFrameEndSec = dNowSec;

	m_WorkingSnapshot.fFrameMs = static_cast<_float>(dFrameIntervalMs);
	m_WorkingSnapshot.fFPS = 0.0 < dFrameIntervalMs ? static_cast<_float>(1000.0 / dFrameIntervalMs) : 0.f;
	m_WorkingSnapshot.CpuMs[ETOUI(EPROFILE_CPU_SECTION::FRAME)] = dWorkMs;

	for (_uint i = 0; i < ETOUI(EPROFILE_CPU_SECTION::END); ++i)
		m_TitleCpuMsAccum[i] += m_WorkingSnapshot.CpuMs[i];
	++m_iTitleFrameCount;

	m_Snapshot = m_WorkingSnapshot;
	m_bFrameOpen = false;

	Update_WindowTitle(dNowSec);
}

_bool CProfiler_Manager::BeginCpuSection(EPROFILE_CPU_SECTION eSection)
{
	if (!m_bFrameOpen)
		return false;

	const _uint iIndex = ETOUI(eSection);
	if (iIndex >= ETOUI(EPROFILE_CPU_SECTION::END))
		return false;

	m_CpuStartSec[iIndex] = NowSec();
	m_bCpuSectionRunning[iIndex] = true;
	return true;
}

void CProfiler_Manager::EndCpuSection(EPROFILE_CPU_SECTION eSection)
{
	const _uint iIndex = ETOUI(eSection);
	if (iIndex >= ETOUI(EPROFILE_CPU_SECTION::END) || !m_bCpuSectionRunning[iIndex])
		return;

	m_WorkingSnapshot.CpuMs[iIndex] += (NowSec() - m_CpuStartSec[iIndex]) * 1000.0;
	m_bCpuSectionRunning[iIndex] = false;
}

void CProfiler_Manager::Set_CurrentPass(EPROFILE_RENDER_PASS ePass)
{
	m_eCurrentPass = ePass;
}

EPROFILE_RENDER_PASS CProfiler_Manager::Get_CurrentPass() const
{
	return m_eCurrentPass;
}

void CProfiler_Manager::AddCounter(EPROFILE_COUNTER eCounter, _uint iValue)
{
	const _uint iIndex = ETOUI(eCounter);
	if (iIndex >= ETOUI(EPROFILE_COUNTER::END))
		return;

	m_WorkingSnapshot.Counters[iIndex] += iValue;
}

void CProfiler_Manager::Record_DrawIndexed(_uint iIndexCount)
{
	(void)iIndexCount;

	AddCounter(EPROFILE_COUNTER::DRAWCALL_TOTAL, 1);

	switch (m_eCurrentPass)
	{
	case EPROFILE_RENDER_PASS::SHADOW:
		AddCounter(EPROFILE_COUNTER::DRAWCALL_SHADOW, 1);
		break;
	case EPROFILE_RENDER_PASS::NONBLEND:
		AddCounter(EPROFILE_COUNTER::DRAWCALL_NONBLEND, 1);
		break;
	case EPROFILE_RENDER_PASS::DECAL:
		AddCounter(EPROFILE_COUNTER::DRAWCALL_DECAL, 1);
		break;
	case EPROFILE_RENDER_PASS::BLEND:
		AddCounter(EPROFILE_COUNTER::DRAWCALL_BLEND, 1);
		break;
	case EPROFILE_RENDER_PASS::UI:
		AddCounter(EPROFILE_COUNTER::DRAWCALL_UI, 1);
		break;
	default:
		break;
	}
}

void CProfiler_Manager::Record_DrawIndexedInstanced(_uint iIndexCount, _uint iInstanceCount)
{
	(void)iInstanceCount;

	Record_DrawIndexed(iIndexCount);
	AddCounter(EPROFILE_COUNTER::DRAWCALL_INSTANCED, 1);
}

void CProfiler_Manager::Set_TextureHubStats(const TEXTURE_HUB_STATS& Stats)
{
	const auto Delta = [](_uint iCurrent, _uint iPrevious) -> _uint
		{
			return iCurrent >= iPrevious ? iCurrent - iPrevious : iCurrent;
		};

	const _uint iReuse = Delta(Stats.iCacheReuseCount, m_PrevTextureHubStats.iCacheReuseCount);
	const _uint iMiss = Delta(Stats.iFirstLoadRequestCount, m_PrevTextureHubStats.iFirstLoadRequestCount);
	const _uint iFailed = Delta(Stats.iLoadFailCount, m_PrevTextureHubStats.iLoadFailCount);

	m_PrevTextureHubStats = Stats;

	if (!m_bFrameOpen)
		return;

	m_WorkingSnapshot.iTextureHubCached = Stats.iCachedSRVCount;
	AddCounter(EPROFILE_COUNTER::TEXTUREHUB_REQUEST, iReuse + iMiss);
	AddCounter(EPROFILE_COUNTER::TEXTUREHUB_REUSE, iReuse);
	AddCounter(EPROFILE_COUNTER::TEXTUREHUB_MISS, iMiss);
	AddCounter(EPROFILE_COUNTER::TEXTUREHUB_FAILED, iFailed);
}

const PROFILER_FRAME_SNAPSHOT& CProfiler_Manager::Get_Snapshot() const
{
	return m_Snapshot;
}

void CProfiler_Manager::Update_WindowTitle(double dNowSec)
{
	const double dElapsedSec = dNowSec - m_dLastTitleUpdateSec;
	if (nullptr == m_hWnd || dElapsedSec < TITLE_UPDATE_INTERVAL)
		return;

	if (0 == m_iTitleFrameCount || dElapsedSec <= 0.0)
	{
		m_dLastTitleUpdateSec = dNowSec;
		return;
	}

	const double dFrameCount = static_cast<double>(m_iTitleFrameCount);
	const double dFPS = dFrameCount / dElapsedSec;
	const double dFrameMs = dElapsedSec * 1000.0 / dFrameCount;
	const double dUpdateMs = m_TitleCpuMsAccum[ETOUI(EPROFILE_CPU_SECTION::UPDATE)] / dFrameCount;
	const double dRenderMs = m_TitleCpuMsAccum[ETOUI(EPROFILE_CPU_SECTION::RENDER_TOTAL)] / dFrameCount;

	m_Snapshot.fAvgFPS = static_cast<_float>(dFPS);

	_tchar szTitle[128] = {};
	swprintf_s(szTitle, TEXT("FPS %.1f | Frame %.1fms | U %.1fms | R %.1fms"),
		dFPS,
		dFrameMs,
		dUpdateMs,
		dRenderMs);

	m_dLastTitleUpdateSec = dNowSec;
	for (_uint i = 0; i < ETOUI(EPROFILE_CPU_SECTION::END); ++i)
		m_TitleCpuMsAccum[i] = 0.0;
	m_iTitleFrameCount = 0;

	SetWindowText(m_hWnd, szTitle);
}

void CProfiler_Manager::Free()
{
	if (g_pProfiler_Manager == this)
		g_pProfiler_Manager = nullptr;

	__super::Free();
}



CScopedCpuProfile::CScopedCpuProfile(EPROFILE_CPU_SECTION eSection)
	: m_eSection(eSection)
{
	CProfiler_Manager* pProfiler = CProfiler_Manager::Get();
	if (nullptr == pProfiler)
		return;

	m_bActive = pProfiler->BeginCpuSection(eSection);
}

CScopedCpuProfile::~CScopedCpuProfile()
{
	if (!m_bActive)
		return;

	if (CProfiler_Manager* pProfiler = CProfiler_Manager::Get())
		pProfiler->EndCpuSection(m_eSection);
}

NS_END