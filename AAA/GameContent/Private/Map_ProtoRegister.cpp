#include "Map_ProtoRegister.h"
#include "GameContent_Log.h"
#include "EnvObject_Static.h"
#include "EnvObject_Interact.h"
#include "EnvTrigger_Generic.h"
#include "EnvTrigger_RenderGlobals.h"
#include "EnvTrigger_EventPublisher.h"
#include "MapSection.h"
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
	if (nullptr == m_pProxy)
		return E_FAIL;

	auto EnsurePrototype = [&](const wchar_t* pPrototypeTag, CGameObject* pPrototype) -> HRESULT
		{
			if (nullptr == pPrototype)
				return E_FAIL;

			if (m_pProxy->Has_Prototype(iObjectLevel, pPrototypeTag))
			{
				Safe_Release(pPrototype);
				return S_OK;
			}

			if (FAILED(m_pProxy->Add_Prototype(iObjectLevel, pPrototypeTag, pPrototype)))
			{
				Safe_Release(pPrototype);
				return E_FAIL;
			}

			return S_OK;
		};

	if (FAILED(EnsurePrototype(CMapSection::PROTOTYPE_TAG, CMapSection::Create(m_pDevice,
		m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CMapStage::PROTOTYPE_TAG, CMapStage::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CEnvObject_Static::PROTOTYPE_TAG,
		CEnvObject_Static::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CEnvObject_Interact::PROTOTYPE_TAG,
		CEnvObject_Interact::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CEnvTrigger_Generic::LEGACY_PROTOTYPE_TAG,
		CEnvTrigger_Generic::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CEnvTrigger_Generic::PROTOTYPE_TAG,
		CEnvTrigger_Generic::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CEnvTrigger_RenderGlobals::PROTOTYPE_TAG,
		CEnvTrigger_RenderGlobals::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(EnsurePrototype(CEnvTrigger_EventPublisher::PROTOTYPE_TAG,
		CEnvTrigger_EventPublisher::Create(m_pDevice, m_pContext))))
		return E_FAIL;

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
			pModelPrototype = CModel::Create_WithTextureHub(
				m_pDevice,
				m_pContext,
				MODEL::MAP,
				wstrModelPath.c_str());
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

//#ifdef _DEBUG
//	if (bCookCollisionMesh)
//	{
//		Log_GameContentInfo(
//			"[EnvCook] begin object="
//			+ WstrToStr(Desc.wstrObjectName)
//			+ " protoTag="
//			+ WstrToStr(Desc.wstrModelProtoTag)
//			+ " path="
//			+ WstrToStr(Desc.wstrModelPath)
//			+ " apxbin="
//			+ WstrToStr(Desc.tCollision.strDecorCollisionApxbinName));
//	}
//#endif

	const string wstrModelPath = WstrToStr(Desc.wstrModelPath);
	CModel* pModelPrototype = nullptr;

	try
	{
		if (bEnablePickingData)
		{
			pModelPrototype = CModel::Create_WithTextureHub(
				m_pDevice,
				m_pContext,
				MODEL::NONANIM,
				wstrModelPath.c_str(),
				XMMatrixIdentity(),
				[](const string&) -> bool
				{
					return true;
				},
				bCookCollisionMesh);
		}
		else
		{
			pModelPrototype = CModel::Create_WithTextureHub(
				m_pDevice,
				m_pContext,
				MODEL::NONANIM,
				wstrModelPath.c_str(),
				XMMatrixIdentity(),
				nullptr,
				bCookCollisionMesh);
		}
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

//#ifdef _DEBUG
//	if (bCookCollisionMesh)
//	{
//		Log_GameContentInfo(
//			"[EnvCook] end object="
//			+ WstrToStr(Desc.wstrObjectName)
//			+ " protoTag="
//			+ WstrToStr(Desc.wstrModelProtoTag));
//	}
//#endif

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