#pragma once

#ifdef _DEBUG

#include <unordered_set>

#include "MapTool_Defines.h"
#include "MapStage.h"

NS_BEGIN(Client)
class CEnvObject;
class CMapStage;
NS_END

NS_BEGIN(MapTool)

struct CULLING_COUNTER
{
	_uint iCandidate = {};
	_uint iVisible = {};
	_uint iCulled = {};
	_uint iSubmitted = {};
};

struct MAPTOOL_PROFILE_FRAME
{
	_uint iFrameIndex = {};

	_uint iMapSectionTotal = {};
	CULLING_COUNTER MapMain = {};
	CULLING_COUNTER MapShadow = {};

	_uint iEnvObjectTotal = {};
	CULLING_COUNTER EnvMain = {};
	CULLING_COUNTER EnvShadow = {};

	_uint iTextureHubUnique = {};
	_uint iTextureHubHit = {};
	_uint iTextureHubMiss = {};
	_uint iTextureHubRawFail = {};

	double dMapCullingCpuMs = {};
};

class CMapToolProfiler final : public CBase
{
	DECLARE_SINGLETON(CMapToolProfiler)

private:
	CMapToolProfiler() = default;
	virtual ~CMapToolProfiler() = default;

public:
	void Set_Stage(Client::CMapStage* pStage);
	void Register_EnvObject(Client::CEnvObject* pEnvObject);
	void Clear_EnvObjects();

	void Update(_float fTimeDelta);
	void Capture_Frame();
	void Reset();

	void Set_Enabled(_bool bEnabled) { m_bEnabled = bEnabled; }
	void Toggle_Enabled() { m_bEnabled = !m_bEnabled; }
	_bool Is_Enabled() const { return m_bEnabled; }

	const MAPTOOL_PROFILE_FRAME& Get_Frame() const { return m_Frame; }
	_bool Has_Stage() const { return nullptr != m_pStage; }

private:
	void Reset_CaptureSession();

private:
	Client::CMapStage* m_pStage = { nullptr };
	unordered_set<Client::CEnvObject*> m_EnvObjects;
	MAPTOOL_PROFILE_FRAME m_Frame = {};

	_bool m_bEnabled = { true };
	_bool m_bFrameBaseValid = { false };
	_uint m_iFrameBase = {};

protected:
	virtual void Free() override;
};

NS_END

#endif