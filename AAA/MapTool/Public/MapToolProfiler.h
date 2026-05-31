#pragma once

#include <fstream>

#include "MapTool_Defines.h"
#include "MapStage.h"

NS_BEGIN(MapTool)

class CMapToolProfiler final : public CBase
{
	DECLARE_SINGLETON(CMapToolProfiler)

private:
	CMapToolProfiler() = default;
	virtual ~CMapToolProfiler() = default;

public:
	void								Set_Stage(Client::CMapStage* pStage);
	void								Update(_float fTimeDelta);
	void								Reset();

	void								Set_Enabled(_bool bEnabled) { m_bEnabled = bEnabled; }
	void								Toggle_Enabled() { m_bEnabled = !m_bEnabled; }
	_bool								Is_Enabled() const { return m_bEnabled; }

	void								Set_CsvEnabled(_bool bEnabled);
	void								Toggle_CsvEnabled() { Set_CsvEnabled(!m_bCsvEnabled); }
	_bool								Is_CsvEnabled() const { return m_bCsvEnabled; }

	const Client::MAP_STAGE_PROFILE&		Get_Frame() const { return m_Frame; }
	const wstring&							Get_CsvPath() const { return m_strCsvPath; }
	_bool									Has_Stage() const { return nullptr != m_pStage; }

private:
	void								Open_CsvIfNeeded();
	void								Close_Csv();
	void								Write_CsvRow();

private:
	Client::CMapStage*					m_pStage = { nullptr };
	Client::MAP_STAGE_PROFILE			m_Frame = {};
	_bool								m_bEnabled = { true };
	_bool								m_bCsvEnabled = { false };
	_float								m_fElapsedSeconds = {};
	wstring								m_strCsvPath = { L"../../Resources/Profile/MapTool_Profile.csv" };
	ofstream							m_Csv;

protected:
	virtual void						Free() override;
};

NS_END
