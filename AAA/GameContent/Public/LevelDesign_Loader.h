#pragma once
#include "Base.h"
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

struct LD_RUNTIME_LOAD_CONTEXT
{
	ID3D11Device* pDevice = { nullptr };
	ID3D11DeviceContext* pContext = { nullptr };

	_uint iPlaceLevel = {};
	_uint iPrototypeLevel = {};

	LD_OBJECT_CREATED_CALLBACK pCreatedCallback = nullptr;
	void* pCallbackContext = nullptr;
};

class CLIENT_DLL CLevelDesign_Loader final : public CBase
{
public:
	CLevelDesign_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevelDesign_Loader() = default;

public:
	HRESULT Initialize();

	static HRESULT Preload_LevelDesign(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strJsonPath, _uint iRuntimeLevel);

	static HRESULT Spawn_LevelDesign(const _wstring& strJsonPath, _uint iRuntimeLevel, LD_LOAD_RESULT* pOutReport = nullptr);

	static HRESULT Load_LevelDesign(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strJsonPath, _uint iRuntimeLevel, LD_LOAD_RESULT* pOutReport = nullptr);

	static HRESULT Load_LevelDesign_Runtime(const LD_RUNTIME_LOAD_CONTEXT& Context, const _wstring& strJsonPath, LD_LOAD_RESULT* pOutReport = nullptr);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pProxy = { nullptr };

private:
	HRESULT Build_Package(const _wstring& strJsonPath, LD_PACKAGE* pOutPackage);
	HRESULT Ready_Prototypes(const LD_RUNTIME_LEVELS& Levels, const LD_PACKAGE& Package);
	HRESULT Spawn(const LD_PACKAGE& Package, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pOutReport = nullptr);

	HRESULT Ensure_ObjectPrototype(_uint iPrototypeLevel, const _wstring& strPrototypeTag);
	HRESULT Spawn_One(const LD_PARSED_OBJECT& Desc, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pInOutReport);

	static void Build_DefaultRuntimeLevels(_uint iRuntimeLevel, LD_RUNTIME_LEVELS* pOutLevels);

	_wstring Make_ObjectTag(const LD_PARSED_OBJECT& Desc) const;

public:
	static CLevelDesign_Loader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END