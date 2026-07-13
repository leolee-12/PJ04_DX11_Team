#include "Map_ProtoRegister.h"
#include "GameContent_Log.h"
#include "EnvObject_Static.h"
#include "EnvObject_Interact.h"
#include "EnvTrigger_Generic.h"
#include "EnvTrigger_RenderGlobals.h"
#include "EnvTrigger_EventPublisher.h"
#include "EnvVolume_Effect.h"
#include "EnvVolume_Culling.h"
#include "EnvVolume_Light.h"
#include "Env_SpotLight.h"
#include "MapSection.h"
#include "MapEvent_BreakWall.h"
#include "MapStage.h"
#include "GameObject_Factory.h"

#include "GameInstance.h"

#include <exception>

namespace
{
	_bool Needs_EnvModelCollisionCook(const ENV_OBJECT_DESC& Desc)
	{
		const ENV_COLLISION_DESC& Collision = Desc.tCollision;

		return Collision.eColliderKind == ENV_COLLIDER_KIND::MODEL_MESH
			&& Collision.bCookCollMesh;
	}

	string ToLower_ASCII(string strValue)
	{
		for (char& ch : strValue)
		{
			if ('A' <= ch && ch <= 'Z')
				ch = ch - 'A' + 'a';
		}

		return strValue;
	}

	_bool Should_CookMapCollisionMesh(const string& strMeshName)
	{
		const string strLowerName = ToLower_ASCII(strMeshName);
		return strLowerName.find("moco") == string::npos;
	}
}

NS_BEGIN(Client)

CMap_ProtoRegister::CMap_ProtoRegister(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pProxy{ CGameInstance::GetProxy() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CMap_ProtoRegister::Ready_Prototypes(const MAP_RUNTIME_LEVELS& Levels, const MAP_PACKAGE&
	Package)
{
	if (FAILED(Ready_ObjectPrototypes(Levels.iObjectLevel)))
		return E_FAIL;

	for (const MAP_SECTION_DESC& Desc : Package.StageDesc.SectionDescs)
	{
		if (FAILED(Ready_MapSectionModel(Levels.iStageModelLevel, Desc)))
			return E_FAIL;
	}

	if (Package.StageDesc.strStageName == CMapEvent_BreakWall::STAGE12_STAGE_NAME)
	{
		MAP_SECTION_DESC Desc{};
		Desc.strSectionName = CMapEvent_BreakWall::STAGE12_SECTION_NAME;
		Desc.wstrModelProtoTag = CMapEvent_BreakWall::STAGE12_MODEL_PROTO_TAG;
		Desc.wstrModelPath = CMapEvent_BreakWall::STAGE12_MODEL_PATH;
		Desc.iModelProtoLevel = Levels.iStageModelLevel;

		if (FAILED(Ready_MapSectionModel(Levels.iStageModelLevel, Desc)))
			return E_FAIL;
	}

	unordered_set<wstring> AllEnvModelTags;
	unordered_set<wstring> CookRequiredEnvModelTags;

	const size_t iTotalEnvDescCount = Package.EnvObjectDescs.size();

	for (const ENV_OBJECT_DESC& Desc : Package.EnvObjectDescs)
	{
		if (Desc.wstrModelProtoTag.empty())
			continue;

		AllEnvModelTags.insert(Desc.wstrModelProtoTag);

		if (Needs_EnvModelCollisionCook(Desc))
			CookRequiredEnvModelTags.insert(Desc.wstrModelProtoTag);
	}

#ifdef _DEBUG
	{
		size_t iHasCollMeshDescCount = 0;
		size_t iCookCollMeshDescCount = 0;
		size_t iUseCollMeshDescCount = 0;

		for (const ENV_OBJECT_DESC& Desc : Package.EnvObjectDescs)
		{
			const ENV_COLLISION_DESC& Collision = Desc.tCollision;

			if (Collision.bHasCollMesh)
				++iHasCollMeshDescCount;

			if (Collision.bCookCollMesh)
				++iCookCollMeshDescCount;

			if (Collision.bUseCollMesh)
				++iUseCollMeshDescCount;
		}

		Log_GameContentInfo(
			"EnvPhysics summary: totalDesc="
			+ to_string(iTotalEnvDescCount)
			+ " hasCollMesh="
			+ to_string(iHasCollMeshDescCount)
			+ " cookCollMesh="
			+ to_string(iCookCollMeshDescCount)
			+ " useCollMesh="
			+ to_string(iUseCollMeshDescCount)
			+ " totalModelTags="
			+ to_string(AllEnvModelTags.size())
			+ " cookModelTags="
			+ to_string(CookRequiredEnvModelTags.size()));
	}
#endif

	unordered_set<wstring> FailedEnvModelTags;

	for (const ENV_OBJECT_DESC& Desc : Package.EnvObjectDescs)
	{
		if (Desc.wstrModelProtoTag.empty())
			continue;

		if (FailedEnvModelTags.find(Desc.wstrModelProtoTag) != FailedEnvModelTags.end())
			continue;

		const _bool bCookCollisionMesh = CookRequiredEnvModelTags.find(Desc.wstrModelProtoTag) != CookRequiredEnvModelTags.end();

		if (FAILED(Ready_EnvModel(
			Levels.iEnvModelLevel,
			Desc,
			bCookCollisionMesh,
			Levels.bEnableEnvObjectPicking)))
		{
			FailedEnvModelTags.insert(Desc.wstrModelProtoTag);

			Log_GameContentWarning(
				"EnvObject model skipped: object="
				+ WstrToStr(Desc.wstrObjectName)
				+ " path=" + WstrToStr(Desc.wstrModelPath));

			continue;
		}
	}

	for (const MAP_ADD_OBJECT& Added : Package.AddedObjectDescs)
	{
		if (m_pProxy->Has_Prototype(Levels.iObjectLevel, Added.strPrototypeTag))
			continue;

		auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(Added.strPrototypeTag);
		if (nullptr == pReg)
			return E_FAIL;

		pReg->ResourceLoader(m_pProxy, m_pDevice, m_pContext, Levels.iObjectLevel);
		if (FAILED(m_pProxy->Add_Prototype(
			Levels.iObjectLevel,
			Added.strPrototypeTag.c_str(),
			static_cast<CGameObject*>(pReg->CreatorFunc(m_pDevice, m_pContext)))))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMap_ProtoRegister::Ready_ObjectPrototypes(_uint iObjectLevel)
{
	auto EnsurePrototype = [&](const wchar_t* pPrototypeTag, auto&& Factory) -> HRESULT
		{
			if (m_pProxy->Has_Prototype(iObjectLevel, pPrototypeTag))
				return S_OK;

			CGameObject* pPrototype = Factory();
			if (nullptr == pPrototype)
				return E_FAIL;

			if (FAILED(m_pProxy->Add_Prototype(iObjectLevel, pPrototypeTag, pPrototype)))
			{
				Safe_Release(pPrototype);
				return E_FAIL;
			}

			return S_OK;
		};

	if (FAILED(EnsurePrototype(CMapSection::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CMapSection::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CMapEvent_BreakWall::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CMapEvent_BreakWall::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CMapStage::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CMapStage::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvObject_Static::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvObject_Static::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvObject_Interact::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvObject_Interact::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvTrigger_Generic::LEGACY_PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvTrigger_Generic::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvTrigger_Generic::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvTrigger_Generic::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvTrigger_RenderGlobals::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvTrigger_RenderGlobals::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvTrigger_EventPublisher::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvTrigger_EventPublisher::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvVolume_Effect::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvVolume_Effect::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvVolume_Culling::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvVolume_Culling::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnvVolume_Light::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnvVolume_Light::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}

	if (FAILED(EnsurePrototype(CEnv_SpotLight::PROTOTYPE_TAG,
		[&]() -> CGameObject* { return CEnv_SpotLight::Create(m_pDevice, m_pContext); })))
	{
		return E_FAIL;
	}


	return S_OK;
}

HRESULT CMap_ProtoRegister::Ready_MapSectionModel(_uint iModelLevel, const MAP_SECTION_DESC& Desc)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	if (!m_pProxy->Has_Prototype(iModelLevel, Desc.wstrModelProtoTag))
	{
		if (Desc.wstrModelPath.empty())
			return E_FAIL;

		const string wstrModelPath = WstrToStr(Desc.wstrModelPath);
		CModel* pModelPrototype = nullptr;

		try
		{
			CModel::MODEL_LOAD_DESC ModelDesc{};
			ModelDesc.eType = MODEL::MAP;
			ModelDesc.pModelFilePath = wstrModelPath.c_str();
			XMStoreFloat4x4(&ModelDesc.PreTransformMatrix, XMMatrixIdentity());
			ModelDesc.fcCollisionCookFilter = &Should_CookMapCollisionMesh;
			pModelPrototype = CModel::Create_WithTextureHub(m_pDevice, m_pContext, ModelDesc);
		}
		catch (const std::exception& e)
		{
			Log_GameContentWarning(
				"MapSection model creation exception: section=" + WstrToStr(Desc.strSectionName)
				+ " path=" + wstrModelPath
				+ " reason=" + e.what());
			return E_FAIL;
		}

		if (nullptr == pModelPrototype)
			return E_FAIL;

		if (FAILED(m_pProxy->Add_Prototype(iModelLevel, Desc.wstrModelProtoTag.c_str(),
			pModelPrototype)))
		{
			Safe_Release(pModelPrototype);
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMap_ProtoRegister::Ready_EnvModel(_uint iModelLevel, const ENV_OBJECT_DESC& Desc, _bool bCookCollisionMesh, _bool bEnablePickingData)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	if (Desc.wstrModelProtoTag.empty() || Desc.wstrModelPath.empty())
		return S_FALSE;

	if (m_pProxy->Has_Prototype(iModelLevel, Desc.wstrModelProtoTag))
		return S_OK;

	const string wstrModelPath = WstrToStr(Desc.wstrModelPath);
	CModel* pModelPrototype = nullptr;

	try
	{
		CModel::MODEL_LOAD_DESC ModelDesc{};
		ModelDesc.eType = MODEL::NONANIM;
		ModelDesc.pModelFilePath = wstrModelPath.c_str();
		XMStoreFloat4x4(&ModelDesc.PreTransformMatrix, XMMatrixIdentity());
		ModelDesc.bCookCollisionMesh = bCookCollisionMesh;

		if (bEnablePickingData)
		{
			ModelDesc.fcPickableFilter = [](const string&) -> bool
				{
					return true;
				};
		}

		pModelPrototype = CModel::Create_WithTextureHub(m_pDevice, m_pContext, ModelDesc);
	}
	catch (const std::exception& e)
	{
		Log_GameContentWarning(
			"EnvObject model creation exception: object="
			+ WstrToStr(Desc.wstrObjectName)
			+ " path="
			+ wstrModelPath
			+ " reason="
			+ e.what());
		return E_FAIL;
	}

	if (nullptr == pModelPrototype)
		return E_FAIL;

	if (FAILED(m_pProxy->Add_Prototype(iModelLevel, Desc.wstrModelProtoTag.c_str(), pModelPrototype)))
	{
		Safe_Release(pModelPrototype);
		return E_FAIL;
	}

	return S_OK;
}

const _tchar* CMap_ProtoRegister::Get_EnvObjectProtoTag(ENV_OBJECT_KIND eKind) const
{
	switch (eKind)
	{
	case ENV_OBJECT_KIND::STATIC: return CEnvObject_Static::PROTOTYPE_TAG;
	case ENV_OBJECT_KIND::INTERACT: return CEnvObject_Interact::PROTOTYPE_TAG;
	case ENV_OBJECT_KIND::EFFECT: return CEnvTrigger_Generic::PROTOTYPE_TAG;
	default: return nullptr;
	}
}

CMap_ProtoRegister* CMap_ProtoRegister::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CMap_ProtoRegister(pDevice, pContext);
}

void CMap_ProtoRegister::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pProxy);

	__super::Free();
}

NS_END