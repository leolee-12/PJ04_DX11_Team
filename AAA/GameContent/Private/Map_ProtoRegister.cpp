#include "Map_ProtoRegister.h"
#include "GameContent_Log.h"
#include "EnvObject_Trigger.h"
#include "EnvObject_Interact.h"
#include "EnvObject_Static.h"
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
			&& Collision.bHasDecorCollisionApxbin
			&& !Collision.bInvalidCollision;
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
		if (Desc.strModelProtoTag.empty())
			continue;

		AllEnvModelTags.insert(Desc.strModelProtoTag);

		if (Needs_EnvModelCollisionCook(Desc))
			CookRequiredEnvModelTags.insert(Desc.strModelProtoTag);
	}

#ifdef _DEBUG
	{
		size_t iCatalogCheckedDescCount = 0;
		size_t iCatalogHitDescCount = 0;
		size_t iModelMeshDescCount = 0;
		size_t iModelMeshCookDescCount = 0;

		for (const ENV_OBJECT_DESC& Desc : Package.EnvObjectDescs)
		{
			if (Desc.tCollision.bCatalogCollisionChecked)
				++iCatalogCheckedDescCount;

			if (Desc.tCollision.bHasDecorCollisionApxbin)
				++iCatalogHitDescCount;

			if (Desc.tCollision.eColliderKind == ENV_COLLIDER_KIND::MODEL_MESH)
			{
				++iModelMeshDescCount;

				if (Needs_EnvModelCollisionCook(Desc))
					++iModelMeshCookDescCount;
			}
		}

		Log_GameContentInfo(
			"EnvPhysics summary: totalDesc="
			+ to_string(iTotalEnvDescCount)
			+ " catalogChecked="
			+ to_string(iCatalogCheckedDescCount)
			+ " catalogHit="
			+ to_string(iCatalogHitDescCount)
			+ " modelMeshDesc="
			+ to_string(iModelMeshDescCount)
			+ " modelMeshCookDesc="
			+ to_string(iModelMeshCookDescCount)
			+ " totalModelTags="
			+ to_string(AllEnvModelTags.size())
			+ " cookModelTags="
			+ to_string(CookRequiredEnvModelTags.size()));
	}
#endif

	for (const ENV_OBJECT_DESC& Desc : Package.EnvObjectDescs)
	{
		if (Desc.strModelProtoTag.empty())
			continue;

		const _bool bCookCollisionMesh =
			CookRequiredEnvModelTags.find(Desc.strModelProtoTag) != CookRequiredEnvModelTags.end();

		if (FAILED(Ready_EnvModel(Levels.iEnvModelLevel, Desc, bCookCollisionMesh, Levels.bEnableEnvObjectPicking)))
			return E_FAIL;
	}

	for (const MAP_ADD_OBJECT& Added : Package.AddedObjectDescs)
	{
		if (m_pProxy->Has_Prototype(Levels.iObjectLevel, Added.strPrototypeTag))
			continue;

		auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(Added.strPrototypeTag);
		if (nullptr == pReg)
			return E_FAIL;

		pReg->ResourceLoader(m_pProxy, m_pDevice, m_pContext);
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

	if (FAILED(EnsurePrototype(CEnvObject_Trigger::PROTOTYPE_TAG,
		CEnvObject_Trigger::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMap_ProtoRegister::Ready_MapSectionModel(_uint iModelLevel, const MAP_SECTION_DESC& Desc)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	if (!m_pProxy->Has_Prototype(iModelLevel, Desc.strModelProtoTag))
	{
		if (Desc.strModelPath.empty())
			return E_FAIL;

		const string strModelPath = WstrToStr(Desc.strModelPath);
		CModel* pModelPrototype = nullptr;

		try
		{
			pModelPrototype = CModel::Create_WithTextureHub(
				m_pDevice,
				m_pContext,
				MODEL::MAP,
				strModelPath.c_str());
		}
		catch (const std::exception& e)
		{
			Log_GameContentWarning(
				"MapSection model creation exception: section=" + WstrToStr(Desc.strSectionName)
				+ " path=" + strModelPath
				+ " reason=" + e.what());
			return E_FAIL;
		}

		if (nullptr == pModelPrototype)
			return E_FAIL;

		if (FAILED(m_pProxy->Add_Prototype(iModelLevel, Desc.strModelProtoTag.c_str(),
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

	if (Desc.strModelProtoTag.empty() || Desc.strModelPath.empty())
		return S_FALSE;

	if (m_pProxy->Has_Prototype(iModelLevel, Desc.strModelProtoTag))
		return S_OK;

//#ifdef _DEBUG
//	if (bCookCollisionMesh)
//	{
//		Log_GameContentInfo(
//			"[EnvCook] begin object="
//			+ WstrToStr(Desc.strObjectName)
//			+ " protoTag="
//			+ WstrToStr(Desc.strModelProtoTag)
//			+ " path="
//			+ WstrToStr(Desc.strModelPath)
//			+ " apxbin="
//			+ WstrToStr(Desc.tCollision.strDecorCollisionApxbinName));
//	}
//#endif

	const string strModelPath = WstrToStr(Desc.strModelPath);
	CModel* pModelPrototype = nullptr;

	try
	{
		if (bEnablePickingData)
		{
			pModelPrototype = CModel::Create_WithTextureHub(
				m_pDevice,
				m_pContext,
				MODEL::NONANIM,
				strModelPath.c_str(),
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
				strModelPath.c_str(),
				XMMatrixIdentity(),
				nullptr,
				bCookCollisionMesh);
		}
	}
	catch (const std::exception& e)
	{
		Log_GameContentWarning(
			"EnvObject model creation exception: object="
			+ WstrToStr(Desc.strObjectName)
			+ " path="
			+ strModelPath
			+ " reason="
			+ e.what());
		return E_FAIL;
	}

//#ifdef _DEBUG
//	if (bCookCollisionMesh)
//	{
//		Log_GameContentInfo(
//			"[EnvCook] end object="
//			+ WstrToStr(Desc.strObjectName)
//			+ " protoTag="
//			+ WstrToStr(Desc.strModelProtoTag));
//	}
//#endif

	if (nullptr == pModelPrototype)
		return E_FAIL;

	if (FAILED(m_pProxy->Add_Prototype(
		iModelLevel,
		Desc.strModelProtoTag.c_str(),
		pModelPrototype)))
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
	case ENV_OBJECT_KIND::EFFECT: return CEnvObject_Trigger::PROTOTYPE_TAG;
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