#include "LevelDesign_ProtoRegister.h"
#include "GameObject_Factory.h"

#include "GameInstance.h"

CLevelDesign_ProtoRegister::CLevelDesign_ProtoRegister(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pProxy{ CGameInstance::GetProxy() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CLevelDesign_ProtoRegister::Ready_Prototypes(const LD_RUNTIME_LEVELS& Levels, const LD_PACKAGE& Package)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	CLevelDesign_Registry::Initialize();

	for (const LD_OBJECT_ENTRY& Desc : Package.ObjectDescs)
	{
		LD_RESOLVED_SPAWN Resolved{};

		if (!CLevelDesign_Registry::Resolve(Desc, &Resolved))
		{
			return E_FAIL;
		}

		if (FAILED(Ensure_Resources(Levels, Resolved.Spec)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevelDesign_ProtoRegister::Ready_PlacementPrototype(const LD_RUNTIME_LEVELS& Levels, const _wstring& strPrototypeTag)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	const LD_SPAWN_SPEC* pSpec = CLevelDesign_Registry::Find_PlacementSpec(strPrototypeTag);
	if (nullptr == pSpec)
		return S_FALSE;

	return Ensure_Resources(Levels, *pSpec);
}

HRESULT CLevelDesign_ProtoRegister::Ensure_Resources(const LD_RUNTIME_LEVELS& Levels, const LD_SPAWN_SPEC& Spec)
{
	if (nullptr == m_pProxy
		|| Spec.strPrototypeTag.empty()
		|| nullptr == Spec.pPrototypeFactory)
	{
		return E_FAIL;
	}

	const _uint iObjectPrototypeLevel = Levels.iPrototypeLevel;
	const _uint iModelPrototypeLevel = Levels.iModelPrototypeLevel;

	if (Spec.bUseFactoryResourceLoader)
	{
		auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(Spec.strPrototypeTag);
		if (nullptr == pReg)
			return E_FAIL;

		pReg->ResourceLoader(m_pProxy, m_pDevice, m_pContext, iObjectPrototypeLevel);
	}

	if (!m_pProxy->Has_Prototype(iObjectPrototypeLevel, Spec.strPrototypeTag))
	{
		CGameObject* pPrototype = Spec.pPrototypeFactory(m_pDevice, m_pContext);

		if (nullptr == pPrototype)
			return E_FAIL;

		if (FAILED(m_pProxy->Add_Prototype(iObjectPrototypeLevel, Spec.strPrototypeTag, pPrototype)))
		{
			Safe_Release(pPrototype);
			return E_FAIL;
		}
	}

	for (const LD_MODEL_REQUIREMENT& Requirement : Spec.ModelRequirements)
	{
		if (m_pProxy->Has_Prototype(iModelPrototypeLevel, Requirement.strPrototypeTag))
			continue;

		CBase* pModel = nullptr;

		if (Requirement.bUseTextureHubLoader)
		{
			CModel::MODEL_LOAD_DESC ModelDesc{};
			ModelDesc.eType = Requirement.eModelType;
			ModelDesc.pModelFilePath = Requirement.strFilePath.c_str();
			ModelDesc.bCookCollisionMesh = Requirement.bCookCollisionMesh;
			ModelDesc.PreTransformMatrix = Requirement.PreTransformMatrix;
			ModelDesc.fcCollisionCookFilter = Requirement.fcCollisionCookFilter;

			pModel = CModel::Create_WithTextureHub(m_pDevice, m_pContext, ModelDesc);
		}
		else
		{
			if (Requirement.bCookCollisionMesh)
				return E_FAIL;

			pModel = CModel::Create(m_pDevice, m_pContext, Requirement.eModelType,
				Requirement.strFilePath.c_str(), XMLoadFloat4x4(&Requirement.PreTransformMatrix));
		}

		if (nullptr == pModel)
			return E_FAIL;

		if (FAILED(m_pProxy->Add_Prototype(iModelPrototypeLevel, Requirement.strPrototypeTag, pModel)))
		{
			Safe_Release(pModel);
			return E_FAIL;
		}
	}

	return S_OK;
}

CLevelDesign_ProtoRegister* CLevelDesign_ProtoRegister::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CLevelDesign_ProtoRegister(pDevice, pContext);
}

void CLevelDesign_ProtoRegister::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pProxy);

	__super::Free();
}