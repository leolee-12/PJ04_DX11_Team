#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"
#include "Map_Override.h"

NS_BEGIN(Client)

class CLIENT_DLL CMap_Loader final : public CBase
{
public:
	CMap_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMap_Loader() = default;

public:
	HRESULT Initialize();

	HRESULT Load_FromManifest(
		const _wstring& strManifestPath,
		const MAP_RUNTIME_LEVELS& Levels,
		const MAP_SPAWN_TARGETS& Targets,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	HRESULT Build_Package(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage);
	HRESULT Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE& Package);
	HRESULT Spawn(const MAP_PACKAGE& Package, const MAP_SPAWN_REQUEST& Request, MAP_LOAD_REPORT* pOutReport = nullptr);

	// Loading 단계에서 MapPackage 준비 -> Ready Prototype -> MapPackage 캐시
	static HRESULT Preload_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel);

	// Level 진입 단게에서 캐시된 Package로 Spawn
	static HRESULT Spawn_Map(
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Spawn_Map_WithOverride(
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		const MAP_OVERRIDE_DESC* pOverrideDesc,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	static HRESULT Preload_MapForLevel(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel);

	static HRESULT Spawn_MapForLevel(
		const _wstring& strFallbackManifestPath,
		const _wstring& strLevelObjectsPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	// Build + Ready + Spawn 한 번에 실행 (편의용)
	static HRESULT Load_Map(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const _wstring& strManifestPath,
		_uint iRuntimeLevel,
		MAP_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	static void Build_DefaultRuntimeLevels(_uint iRuntimeLevel, MAP_RUNTIME_LEVELS* pOutLevels);
	static void Build_DefaultRuntimeTargets(_uint iRuntimeLevel, MAP_SPAWN_TARGETS* pOutTargets);

public:
	static CMap_Loader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END