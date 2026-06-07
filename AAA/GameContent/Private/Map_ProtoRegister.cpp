#include "Map_ProtoRegister.h"

#include "EnvObject_Effect.h"
#include "EnvObject_Interact.h"
#include "EnvObject_Static.h"
#include "GameContent_Log.h"
#include "GameInstance.h"
#include "MapSection.h"
#include "MapStage.h"
#include "Model.h"

#include <exception>

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

	for (const ENV_OBJECT_DESC& Desc : Package.EnvObjectDescs)
	{
		if (Desc.strModelProtoTag.empty())
			continue;

		if (FAILED(Ready_EnvModel(Levels.iEnvModelLevel, Desc)))
			return E_FAIL;
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

	if (FAILED(EnsurePrototype(CEnvObject_Effect::PROTOTYPE_TAG,
		CEnvObject_Effect::Create(m_pDevice, m_pContext))))
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

HRESULT CMap_ProtoRegister::Ready_EnvModel(_uint iModelLevel, const ENV_OBJECT_DESC& Desc)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	if (Desc.strModelProtoTag.empty() || Desc.strModelPath.empty())
		return S_FALSE;

	if (m_pProxy->Has_Prototype(iModelLevel, Desc.strModelProtoTag))
		return S_OK;

	const string strModelPath = WstrToStr(Desc.strModelPath);
	CModel* pModelPrototype = nullptr;

	try
	{
		pModelPrototype = CModel::Create_WithTextureHub(
			m_pDevice,
			m_pContext,
			MODEL::NONANIM,
			strModelPath.c_str());
	}
	catch (const std::exception& e)
	{
		Log_GameContentWarning(
			"EnvObject model creation exception: object=" + WstrToStr(Desc.strObjectName)
			+ " path=" + strModelPath
			+ " reason=" + e.what());
		return E_FAIL;
	}

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
	case ENV_OBJECT_KIND::EFFECT: return CEnvObject_Effect::PROTOTYPE_TAG;
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