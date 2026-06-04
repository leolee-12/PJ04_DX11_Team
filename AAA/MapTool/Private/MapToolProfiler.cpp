#include "MapToolProfiler.h"

#ifdef _DEBUG

#include <fstream>

IMPLEMENT_SINGLETON(CMapToolProfiler)

namespace
{
	constexpr _float CSV_WRITE_INTERVAL_SECONDS = 0.25f;
}

void CMapToolProfiler::Set_Stage(Client::CMapStage* pStage)
{
	if (m_pStage == pStage)
		return;

	m_pStage = pStage;
	Reset_CaptureSession();
}

void CMapToolProfiler::Update(_float fTimeDelta)
{
	if (!m_bEnabled)
		return;

	if (nullptr == m_pStage)
	{
		m_Frame = {};
		m_bFrameBaseValid = false;
		m_fCsvWriteAccumulator = 0.f;
		return;
	}

	m_fElapsedSeconds += fTimeDelta;
	if (m_bCsvEnabled)
		m_fCsvWriteAccumulator += fTimeDelta;
}

void CMapToolProfiler::Capture_Frame()
{
	if (!m_bEnabled || nullptr == m_pStage)
		return;

	const Client::MAP_STAGE_PROFILE RawFrame = m_pStage->Get_Profile();
	Client::MAP_STAGE_PROFILE DisplayFrame = RawFrame;
	DisplayFrame.dSectionRenderCpuMs = 0.0;
	DisplayFrame.iEstimatedDrawCalls = {};

	for (Client::CMapSection* pSection : m_pStage->Get_Sections())
	{
		if (nullptr == pSection)
			continue;

		const Client::MAP_SECTION_PROFILE& SectionProfile = pSection->Get_Profile();
		DisplayFrame.dSectionRenderCpuMs += SectionProfile.dRenderCpuMs;
		DisplayFrame.iEstimatedDrawCalls += SectionProfile.iEstimatedDrawCalls;
	}

	if (!m_bFrameBaseValid)
	{
		m_iFrameBase = DisplayFrame.iFrameIndex;
		m_bFrameBaseValid = true;
	}

	m_Frame = DisplayFrame;
	m_Frame.iFrameIndex = (DisplayFrame.iFrameIndex >= m_iFrameBase) ? (DisplayFrame.iFrameIndex - m_iFrameBase) : 0;

	if (m_bCsvEnabled)
	{
		if (!m_bCsvHasWrittenRow || m_fCsvWriteAccumulator >= CSV_WRITE_INTERVAL_SECONDS)
		{
			Write_CsvRow();
			if (m_bCsvEnabled)
				m_bCsvHasWrittenRow = true;
			m_fCsvWriteAccumulator = 0.f;
		}
	}
}

void CMapToolProfiler::Reset_CaptureSession()
{
	m_Frame = {};
	m_iFrameBase = {};
	m_bFrameBaseValid = false;
	m_fElapsedSeconds = 0.f;
	m_fCsvWriteAccumulator = 0.f;
	m_bCsvHasWrittenRow = false;

	if (m_bCsvEnabled)
		Close_Csv();
}

void CMapToolProfiler::Reset()
{
	Reset_CaptureSession();
}

void CMapToolProfiler::Set_CsvEnabled(_bool bEnabled)
{
	if (m_bCsvEnabled == bEnabled)
	{
		if (m_bCsvEnabled)
			m_bEnabled = true;
		return;
	}

	m_bCsvEnabled = bEnabled;

	if (m_bCsvEnabled)
	{
		m_bEnabled = true;
		m_fCsvWriteAccumulator = 0.f;
		m_bCsvHasWrittenRow = false;
		Close_Csv();
	}
	else
	{
		Close_Csv();
	}
}

void CMapToolProfiler::Open_CsvIfNeeded()
{
	if (m_Csv.is_open())
		return;

	CreateDirectoryW(L"../../Resources/Profile", nullptr);

	const string strPath = WstrToStr(m_strCsvPath);
	m_Csv.open(strPath, ios::out | ios::trunc);
	if (!m_Csv.is_open())
	{
		m_bCsvEnabled = false;
		return;
	}

	m_Csv
		<< "elapsed_sec,frame,total,visible,culled,submitted_nonblend,submitted_blend,submitted_shadow,"
		<< "stage_late_ms,culling_ms,section_render_ms,estimated_draw_calls\n";
}

void CMapToolProfiler::Close_Csv()
{
	if (m_Csv.is_open())
		m_Csv.close();
}

void CMapToolProfiler::Write_CsvRow()
{
	if (!m_bEnabled || !m_bCsvEnabled || nullptr == m_pStage)
		return;

	Open_CsvIfNeeded();
	if (!m_Csv.is_open())
		return;

	m_Csv
		<< m_fElapsedSeconds << ','
		<< m_Frame.iFrameIndex << ','
		<< m_Frame.iTotalSections << ','
		<< m_Frame.iVisibleSections << ','
		<< m_Frame.iCulledSections << ','
		<< m_Frame.iSubmittedNonBlend << ','
		<< m_Frame.iSubmittedBlend << ','
		<< m_Frame.iSubmittedShadow << ','
		<< m_Frame.dStageLateUpdateCpuMs << ','
		<< m_Frame.dCullingCpuMs << ','
		<< m_Frame.dSectionRenderCpuMs << ','
		<< m_Frame.iEstimatedDrawCalls << '\n';
}

void CMapToolProfiler::Free()
{
	Close_Csv();
	m_pStage = nullptr;

	__super::Free();
}

#endif
