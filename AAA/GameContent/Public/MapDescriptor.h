#pragma once

#include "Base.h"
#include "Map_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)
class CMapStage;

class CLIENT_DLL CMapDescriptor final : public CBase
{
	DECLARE_SINGLETON(CMapDescriptor)

public:
	struct MAP_PRESET_LOAD_REPORT
	{
		_bool		bStageLoaded = { false };
		_wstring	strStageName = {};
		_uint		iEnvJsonLoadedCount = {};
		_uint		iEnvJsonFailedCount = {};
		_uint		iEnvDescriptorCount = {};
		_uint		iEnvCreatedCount = {};
		_uint		iEnvSkippedMissingModel = {};
		_uint		iEnvSkippedCreateFailed = {};
	};

	using MAP_PRESET_OBJECT_CREATED_CALLBACK = void(*)(void* pContext, CGameObject* pObject,
		const wstring& strPrototypeTag, const wstring& strLayerTag, const wstring& strObjectTag);

public:
	CMapDescriptor() = default;
	virtual ~CMapDescriptor() = default;

public:
	HRESULT Initialize();
	HRESULT Bulid_MapStageDesc(_uint iPresetIndex, _uint iSectionProtoLevel, _uint iModelLevel, MAP_STAGE_DESC* pOutStageDesc);
	HRESULT Load_MapStage(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		_uint iPresetIndex,
		_uint iPlaceLevel,
		_uint iModelLevel,
		MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
		void* pCallbackContext = nullptr,
		CMapStage** ppOutStage = nullptr,
		_wstring* pOutStageName = nullptr);
	HRESULT Load_EnvObject_FromJson(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		_uint iPresetIndex,
		_uint iPlaceLevel,
		MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
		void* pCallbackContext = nullptr,
		MAP_PRESET_LOAD_REPORT* pOutReport = nullptr);
	HRESULT Load_MapPreset(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		_uint iPresetIndex,
		_uint iPlaceLevel,
		_uint iModelLevel,
		MAP_PRESET_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr,
		void* pCallbackContext = nullptr,
		MAP_PRESET_LOAD_REPORT* pOutReport = nullptr,
		CMapStage** ppOutStage = nullptr);

	_bool			Is_MapLayer(const wstring& strLayerTag) const;
	_uint			Get_MapPresetCount() const;
	const _char*	Get_MapPresetLabel(_uint iPresetIndex) const;

private:
	HRESULT Ready_MapStagePrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iPlaceLevel);
	HRESULT Ensure_Initialized();

private:
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	virtual void Free() override;
};

NS_END
