#include "MapToolProfiler.h"

#include <fstream>

IMPLEMENT_SINGLETON(CMapToolProfiler)

void CMapToolProfiler::Set_Stage(Client::CMapStage* pStage)
{
	m_pStage = pStage;
}

void CMapToolProfiler::Update(_float fTimeDelta)
{
	if (!m_bEnabled)
		return;

	m_fElapsedSeconds += fTimeDelta;

	if (nullptr != m_pStage)
		m_Frame = m_pStage->Get_Profile();
	else
		m_Frame = {};

	if (m_bCsvEnabled)
		Write_CsvRow();
}

void CMapToolProfiler::Reset()
{
	m_Frame = {};
	m_fElapsedSeconds = 0.f;
}

void CMapToolProfiler::Set_CsvEnabled(_bool bEnabled)
{
	if (m_bCsvEnabled == bEnabled)
		return;

	m_bCsvEnabled = bEnabled;

	if (m_bCsvEnabled)
		Open_CsvIfNeeded();
	else
		Close_Csv();
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
		<< "stage_late_ms,culling_ms,section_render_ms,estimated_draw_calls,estimated_triangles\n";
}

void CMapToolProfiler::Close_Csv()
{
	if (m_Csv.is_open())
		m_Csv.close();
}

void CMapToolProfiler::Write_CsvRow()
{
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
		<< m_Frame.iEstimatedDrawCalls << ','
		<< m_Frame.iEstimatedTriangles << '\n';
}

void CMapToolProfiler::Free()
{
	Close_Csv();
	m_pStage = nullptr;

	__super::Free();
}
