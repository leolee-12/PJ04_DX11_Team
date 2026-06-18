#include "LevelDesign_Loader.h"
#include "LevelDesign_Parser.h"
#include "LevelDesign_Registry.h"
#include "LevelDesign_Unsupported.h"
#include "LevelDesign_Breakable.h"

#include "GameInstance.h"

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

	LD_BREAKABLE_TYPE Resolve_LevelDesignBreakableType(const _wstring& strObjectName)
	{
		if (strObjectName == L"StarBlock" || strObjectName == L"StarBlockBig")
			return LD_BREAKABLE_TYPE::STAR_BLOCK;

		return LD_BREAKABLE_TYPE::END;
	}

	const _tchar* Resolve_LevelDesignBreakableModelProtoTag(const _wstring& strObjectName)
	{
		if (strObjectName == L"StarBlock")
			return CLevelDesign_Breakable::STARBLOCK_H1W1_MODEL_PROTO_TAG;

		if (strObjectName == L"StarBlockBig")
			return CLevelDesign_Breakable::STARBLOCK_H3W3_MODEL_PROTO_TAG; // 임시 가정

		return nullptr;
	}

	const LD_SPAWN_SPEC& Resolve_LevelDesignSpawnSpec(const LD_PARSED_OBJECT& Desc)
	{
		const LD_SPAWN_SPEC* pSpec = CLevelDesign_Registry::Find(Desc.strObjectName);
		if (nullptr == pSpec)
			return CLevelDesign_Registry::Get_FallbackSpec();

		if (pSpec->strPrototypeTag == CLevelDesign_Breakable::PROTOTYPE_TAG)
		{
			const LD_BREAKABLE_TYPE eBreakableType = Resolve_LevelDesignBreakableType(Desc.strObjectName);

			if (eBreakableType != LD_BREAKABLE_TYPE::STAR_BLOCK)
				return CLevelDesign_Registry::Get_FallbackSpec();
		}

		return *pSpec;
	}

	void Build_LevelDesignBreakableDesc(const LD_PARSED_OBJECT& Src, LD_BREAKABLE_OBJECT_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return;

		*pOutDesc = {};
		static_cast<LD_COMMON_DESC&>(*pOutDesc) = static_cast<const LD_COMMON_DESC&>(Src);

		pOutDesc->eCategory = LD_CATEGORY::BREAKABLE;
		pOutDesc->eType = Resolve_LevelDesignBreakableType(Src.strObjectName);
		pOutDesc->wstrModelProtoTag.clear();

		if (const _tchar* pModelProtoTag = Resolve_LevelDesignBreakableModelProtoTag(Src.strObjectName))
			pOutDesc->wstrModelProtoTag = pModelProtoTag;
	}
}

NS_BEGIN(Client)

CLevelDesign_Loader::CLevelDesign_Loader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pProxy{ CGameInstance::GetProxy() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CLevelDesign_Loader::Initialize()
{
	return nullptr != m_pProxy ? S_OK : E_FAIL;
}

HRESULT CLevelDesign_Loader::Preload_LevelDesign(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const _wstring& strJsonPath,
	_uint iRuntimeLevel)
{
	if (nullptr == pDevice || nullptr == pContext || strJsonPath.empty())
		return E_FAIL;

	CLevelDesign_Loader* pLoader = Create(pDevice, pContext);
	if (nullptr == pLoader)
		return E_FAIL;

	LD_PACKAGE Package{};
	HRESULT hr = pLoader->Build_Package(strJsonPath, &Package);

	if (SUCCEEDED(hr))
	{
		LD_RUNTIME_LEVELS Levels{};
		Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);
		hr = pLoader->Ready_Prototypes(Levels, Package);
	}

	Safe_Release(pLoader);
	return hr;
}

HRESULT CLevelDesign_Loader::Spawn_LevelDesign(
	const _wstring& strJsonPath,
	_uint iRuntimeLevel,
	LD_LOAD_RESULT* pOutReport)
{
	UNREFERENCED_PARAMETER(strJsonPath);
	UNREFERENCED_PARAMETER(iRuntimeLevel);

	if (nullptr != pOutReport)
		*pOutReport = {};

	return E_NOTIMPL;
}

HRESULT CLevelDesign_Loader::Load_LevelDesign(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const _wstring& strJsonPath,
	_uint iRuntimeLevel,
	LD_LOAD_RESULT* pOutReport)
{
	if (nullptr == pDevice || nullptr == pContext || strJsonPath.empty())
		return E_FAIL;

	if (nullptr != pOutReport)
		*pOutReport = {};

	CLevelDesign_Loader* pLoader = Create(pDevice, pContext);
	if (nullptr == pLoader)
		return E_FAIL;

	LD_PACKAGE Package{};
	HRESULT hr = pLoader->Build_Package(strJsonPath, &Package);

	if (SUCCEEDED(hr))
	{
		LD_RUNTIME_LEVELS Levels{};
		Build_DefaultRuntimeLevels(iRuntimeLevel, &Levels);

		hr = pLoader->Ready_Prototypes(Levels, Package);
		if (SUCCEEDED(hr))
		{
			LD_SPAWN_REQUEST Request{};
			Request.Levels = Levels;
			Request.iPlaceLevel = iRuntimeLevel;

			hr = pLoader->Spawn(Package, Request, pOutReport);
		}
	}

	Safe_Release(pLoader);
	return hr;
}

HRESULT CLevelDesign_Loader::Load_LevelDesign_Runtime(const LD_RUNTIME_LOAD_CONTEXT& Context, const _wstring& strJsonPath, LD_LOAD_RESULT* pOutReport)
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

	if (SUCCEEDED(hr))
	{
		LD_RUNTIME_LEVELS Levels{};
		Levels.iObjectLevel = Context.iPlaceLevel;
		Levels.iPrototypeLevel = Context.iPrototypeLevel;

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
	if (nullptr == m_pProxy)
		return E_FAIL;

	CLevelDesign_Registry::Initialize();

	unordered_set<_wstring> RequiredPrototypes;
	RequiredPrototypes.reserve(Package.ObjectDescs.size());

	for (const LD_PARSED_OBJECT& Desc : Package.ObjectDescs)
	{
		const LD_SPAWN_SPEC& Spec = Resolve_LevelDesignSpawnSpec(Desc);

		if (!Spec.strPrototypeTag.empty())
			RequiredPrototypes.emplace(Spec.strPrototypeTag);
	}

	for (const _wstring& strPrototypeTag : RequiredPrototypes)
	{
		if (FAILED(Ensure_ObjectPrototype(Levels.iPrototypeLevel, strPrototypeTag)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Loader::Spawn(const LD_PACKAGE& Package, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pOutReport)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	if (nullptr != pOutReport)
	{
		*pOutReport = {};
		pOutReport->wstrSourcePath = Package.wstrSourcePath;
		pOutReport->iParsedObjectCount = static_cast<_uint>(Package.ObjectDescs.size());
		pOutReport->iSpawnCandidateCount = static_cast<_uint>(Package.ObjectDescs.size());
	}

	for (const LD_PARSED_OBJECT& Desc : Package.ObjectDescs)
	{
		if (FAILED(Spawn_One(Desc, Request, pOutReport)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Loader::Ensure_ObjectPrototype(_uint iPrototypeLevel, const _wstring& strPrototypeTag)
{
	if (nullptr == m_pProxy || strPrototypeTag.empty())
		return E_FAIL;

	auto EnsureObjectPrototype = [&](const _tchar* pPrototypeTag, CGameObject* pPrototype) -> HRESULT
		{
			if (nullptr == pPrototype)
				return E_FAIL;

			if (m_pProxy->Has_Prototype(iPrototypeLevel, pPrototypeTag))
			{
				Safe_Release(pPrototype);
				return S_OK;
			}

			if (FAILED(m_pProxy->Add_Prototype(iPrototypeLevel, pPrototypeTag, pPrototype)))
			{
				Safe_Release(pPrototype);
				return E_FAIL;
			}

			return S_OK;
		};

	auto EnsureModelPrototype = [&](const _wstring& strModelProtoTag, const _char* pModelPath) -> HRESULT
		{
			if (strModelProtoTag.empty() || nullptr == pModelPath)
				return E_FAIL;

			if (m_pProxy->Has_Prototype(ETOUI(LEVEL::GAMEPLAY), strModelProtoTag))
				return S_OK;

			CBase* pModelPrototype = CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, pModelPath);
			if (nullptr == pModelPrototype)
				return E_FAIL;

			if (FAILED(m_pProxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), strModelProtoTag, pModelPrototype)))
			{
				Safe_Release(pModelPrototype);
				return E_FAIL;
			}

			return S_OK;
		};

	if (strPrototypeTag == CLevelDesign_Unsupported::PROTOTYPE_TAG)
	{
		if (m_pProxy->Has_Prototype(iPrototypeLevel, strPrototypeTag))
			return S_OK;

		return EnsureObjectPrototype(
			CLevelDesign_Unsupported::PROTOTYPE_TAG,
			CLevelDesign_Unsupported::Create(m_pDevice, m_pContext));
	}

	if (strPrototypeTag == CLevelDesign_Breakable::PROTOTYPE_TAG)
	{
		if (!m_pProxy->Has_Prototype(iPrototypeLevel, strPrototypeTag))
		{
			if (FAILED(EnsureObjectPrototype(
				CLevelDesign_Breakable::PROTOTYPE_TAG,
				CLevelDesign_Breakable::Create(m_pDevice, m_pContext))))
			{
				return E_FAIL;
			}
		}

		if (FAILED(EnsureModelPrototype(
			CLevelDesign_Breakable::STARBLOCK_H1W1_MODEL_PROTO_TAG,
			"../../Resources/Map/Gimmick/Star/H1W1.ysh")))
		{
			return E_FAIL;
		}

		if (FAILED(EnsureModelPrototype(
			CLevelDesign_Breakable::STARBLOCK_H3W3_MODEL_PROTO_TAG,
			"../../Resources/Map/Gimmick/Star/H3W3.ysh")))
		{
			return E_FAIL;
		}

		return S_OK;
	}

	return E_FAIL;
}

HRESULT CLevelDesign_Loader::Spawn_One(const LD_PARSED_OBJECT& Desc, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pInOutReport)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	const LD_SPAWN_SPEC& FallbackSpec = CLevelDesign_Registry::Get_FallbackSpec();
	const LD_SPAWN_SPEC& Spec = Resolve_LevelDesignSpawnSpec(Desc);

	if (&Spec == &FallbackSpec)
	{
		if (nullptr != pInOutReport)
			++pInOutReport->iFallbackSpecCount;
	}

	if (Spec.strPrototypeTag.empty() || Spec.strLayerTag.empty())
		return E_FAIL;

	const _wstring strObjectTag = Make_ObjectTag(Desc);

	CGameObject* pCreatedObject = nullptr;
	HRESULT hr = E_FAIL;

	if (Spec.strPrototypeTag == CLevelDesign_Breakable::PROTOTYPE_TAG)
	{
		LD_BREAKABLE_OBJECT_DESC BreakableDesc{};
		Build_LevelDesignBreakableDesc(Desc, &BreakableDesc);

		hr = m_pProxy->Add_GameObject_Return(
			&pCreatedObject,
			Request.Levels.iPrototypeLevel,
			Spec.strPrototypeTag,
			Request.iPlaceLevel,
			Spec.strLayerTag,
			strObjectTag,
			&BreakableDesc);
	}
	else
	{
		LD_PARSED_OBJECT SpawnDesc = Desc;
		SpawnDesc.eCategory = Spec.eCategory;

		hr = m_pProxy->Add_GameObject_Return(
			&pCreatedObject,
			Request.Levels.iPrototypeLevel,
			Spec.strPrototypeTag,
			Request.iPlaceLevel,
			Spec.strLayerTag,
			strObjectTag,
			&SpawnDesc);
	}

	if (FAILED(hr) || nullptr == pCreatedObject)
	{
		if (nullptr != pInOutReport)
			++pInOutReport->iSkippedCreateFailedCount;

		return E_FAIL;
	}

	if (nullptr != pInOutReport)
		++pInOutReport->iCreatedCount;

	if (nullptr != Request.pCreatedCallback)
	{
		Request.pCreatedCallback(
			Request.pCallbackContext,
			pCreatedObject,
			Spec.strPrototypeTag,
			Spec.strLayerTag,
			strObjectTag);
	}

	return S_OK;
}

void CLevelDesign_Loader::Build_DefaultRuntimeLevels(_uint iRuntimeLevel, LD_RUNTIME_LEVELS* pOutLevels)
{
	UNREFERENCED_PARAMETER(iRuntimeLevel);

	if (nullptr == pOutLevels)
		return;

	*pOutLevels = {};
	pOutLevels->iObjectLevel = ETOUI(LEVEL::STATIC);
	pOutLevels->iPrototypeLevel = ETOUI(LEVEL::STATIC);
}

_wstring CLevelDesign_Loader::Make_ObjectTag(const LD_PARSED_OBJECT& Desc) const
{
	if (Desc.iUid != 0)
		return Desc.strObjectName + L"_" + to_wstring(Desc.iUid);

	if (!Desc.strEntryKey.empty())
		return Desc.strObjectName + L"_" + Desc.strEntryKey;

	return Desc.strObjectName;
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
	Safe_Release(m_pProxy);

	__super::Free();
}

NS_END