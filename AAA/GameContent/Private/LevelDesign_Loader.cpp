#include "LevelDesign_Loader.h"
#include "LevelDesign_Parser.h"
#include "LevelDesign_Spawner.h"
#include "LevelDesign_ProtoRegister.h"
#include "Map_EditFile.h"

#include <mutex>

namespace
{
	mutex g_LevelDesignPackageCacheMutex;
	unordered_map<_wstring, LD_PACKAGE> g_LevelDesignPackageCache;

	_wstring Make_LevelDesignCacheKey(const _wstring& strJsonPath)
	{
		return strJsonPath;
	}

	void Store_LevelDesignPackage(const _wstring& strJsonPath, const LD_PACKAGE& Package)
	{
		lock_guard<mutex> lock(g_LevelDesignPackageCacheMutex);
		g_LevelDesignPackageCache[Make_LevelDesignCacheKey(strJsonPath)] = Package;
	}

	_bool Try_GetLevelDesignPackage(const _wstring& strJsonPath, LD_PACKAGE* pOutPackage)
	{
		if (nullptr == pOutPackage)
			return false;

		lock_guard<mutex> lock(g_LevelDesignPackageCacheMutex);

		const auto Iter = g_LevelDesignPackageCache.find(Make_LevelDesignCacheKey(strJsonPath));
		if (Iter == g_LevelDesignPackageCache.end())
			return false;

		*pOutPackage = Iter->second;
		return true;
	}

	_bool Is_RuntimeLoadContextValid(const LD_RUNTIME_LOAD_CONTEXT& Context)
	{
		return nullptr != Context.pDevice
			&& nullptr != Context.pContext;
	}
}

NS_BEGIN(Client)

CLevelDesign_Loader::CLevelDesign_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CLevelDesign_Loader::Initialize()
{
	return S_OK;
}

HRESULT CLevelDesign_Loader::Preload_LevelDesign(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strJsonPath, _uint iRuntimeLevel)
{
	LD_RUNTIME_LEVELS Levels{};
	Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);
	return Preload_LevelDesign(pDevice, pContext, strJsonPath, Levels);
}

HRESULT CLevelDesign_Loader::Preload_LevelDesign(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strJsonPath, const LD_RUNTIME_LEVELS& Levels)
{
	if (nullptr == pDevice || nullptr == pContext || strJsonPath.empty())
		return E_FAIL;

	CLevelDesign_Loader* pLoader = Create(pDevice, pContext);
	if (nullptr == pLoader)
		return E_FAIL;

	LD_PACKAGE Package{};
	HRESULT hr = pLoader->Build_Package(strJsonPath, &Package);

	if (SUCCEEDED(hr))
		hr = pLoader->Ready_Prototypes(Levels, Package);

	Safe_Release(pLoader);
	return hr;
}

HRESULT CLevelDesign_Loader::Load_LevelDesign_Runtime(const LD_RUNTIME_LOAD_CONTEXT& Context, const _wstring& strJsonPath,
	LD_LOAD_RESULT* pOutReport, const MAP_EDIT_CHANGE* pOverrideDesc)
{
	if (nullptr != pOutReport)
		*pOutReport = {};

	if (!Is_RuntimeLoadContextValid(Context) || strJsonPath.empty())
		return E_FAIL;

	CLevelDesign_Loader* pLoader = Create(Context.pDevice, Context.pContext);
	if (nullptr == pLoader)
		return E_FAIL;

	LD_PACKAGE Package{};
	HRESULT hr = pLoader->Build_Package(strJsonPath, &Package);

	if (SUCCEEDED(hr) && nullptr != pOverrideDesc)
		hr = CMap_EditFile::Apply_LevelDesignChange(&Package, *pOverrideDesc);

	if (SUCCEEDED(hr))
	{
		LD_RUNTIME_LEVELS Levels{};
		Levels.iObjectLevel = Context.iPlaceLevel;
		Levels.iPrototypeLevel = Context.iPrototypeLevel;
		Levels.iModelPrototypeLevel = Context.iModelPrototypeLevel;

		hr = pLoader->Ready_Prototypes(Levels, Package);
		if (SUCCEEDED(hr))
		{
			LD_SPAWN_REQUEST Request{};
			Request.Levels = Levels;
			Request.iPlaceLevel = Context.iPlaceLevel;
			Request.pCreatedCallback = Context.pCreatedCallback;
			Request.pCallbackContext = Context.pCallbackContext;

			hr = pLoader->Spawn(Package, Request, pOutReport);
		}
	}

	Safe_Release(pLoader);
	return hr;
}

HRESULT CLevelDesign_Loader::Build_Package(const _wstring& strJsonPath, LD_PACKAGE* pOutPackage)
{
	if (nullptr == pOutPackage || strJsonPath.empty())
		return E_FAIL;

	if (Try_GetLevelDesignPackage(strJsonPath, pOutPackage))
		return S_OK;

	LD_PACKAGE Package{};
	if (FAILED(CLevelDesign_Parser::Parse_File(strJsonPath, &Package)))
		return E_FAIL;

	Store_LevelDesignPackage(strJsonPath, Package);
	*pOutPackage = Package;
	return S_OK;
}

HRESULT CLevelDesign_Loader::Ready_Prototypes(const LD_RUNTIME_LEVELS& Levels, const LD_PACKAGE& Package)
{
	CLevelDesign_ProtoRegister* pRegister = CLevelDesign_ProtoRegister::Create(m_pDevice, m_pContext);

	if (nullptr == pRegister)
		return E_FAIL;

	const HRESULT hr = pRegister->Ready_Prototypes(Levels, Package);

	Safe_Release(pRegister);
	return hr;
}

HRESULT CLevelDesign_Loader::Spawn(const LD_PACKAGE& Package, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pOutReport)
{
	CLevelDesign_Spawner* pSpawner =
		CLevelDesign_Spawner::Create();

	if (nullptr == pSpawner)
		return E_FAIL;

	const HRESULT hr =
		pSpawner->Spawn(Package, Request, pOutReport);

	Safe_Release(pSpawner);
	return hr;
}

void CLevelDesign_Loader::Build_DefaultRuntimeLevels(_uint iRuntimeLevel, LD_RUNTIME_LEVELS* pOutLevels)
{
	if (nullptr == pOutLevels)
		return;

	*pOutLevels = {};
	pOutLevels->iObjectLevel = iRuntimeLevel;
	pOutLevels->iPrototypeLevel = iRuntimeLevel;
	pOutLevels->iModelPrototypeLevel = iRuntimeLevel;
}

CLevelDesign_Loader* CLevelDesign_Loader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Loader* pInstance = new CLevelDesign_Loader(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Loader");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Loader::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	__super::Free();
}

NS_END