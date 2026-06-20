#include "LevelDesign_ProtoRegister.h"
#include "Model.h"
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

	unordered_set<_wstring> RequiredPrototypes;
	RequiredPrototypes.reserve(Package.ObjectDescs.size());

	for (const LD_PARSED_OBJECT& Desc : Package.ObjectDescs)
	{
		LD_RESOLVED_SPAWN Resolved{};

		if (!CLevelDesign_Registry::Resolve(
			Desc,
			&Resolved))
		{
			return E_FAIL;
		}

		if (FAILED(Ensure_Resources(
			Levels.iPrototypeLevel,
			Resolved.Spec)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevelDesign_ProtoRegister::Ensure_Resources(_uint iPrototypeLevel, const LD_SPAWN_SPEC& Spec)
{
	if (nullptr == m_pProxy
		|| Spec.strPrototypeTag.empty()
		|| nullptr == Spec.pPrototypeFactory)
	{
		return E_FAIL;
	}

	if (!m_pProxy->Has_Prototype(iPrototypeLevel, Spec.strPrototypeTag))
	{
		CGameObject* pPrototype = Spec.pPrototypeFactory(m_pDevice, m_pContext);

		if (nullptr == pPrototype)
			return E_FAIL;

		if (FAILED(m_pProxy->Add_Prototype(iPrototypeLevel, Spec.strPrototypeTag, pPrototype)))
		{
			Safe_Release(pPrototype);
			return E_FAIL;
		}
	}

	for (const LD_MODEL_REQUIREMENT& Requirement : Spec.ModelRequirements)
	{
		if (m_pProxy->Has_Prototype(Requirement.iPrototypeLevel, Requirement.strPrototypeTag))
		{
			continue;
		}

		CBase* pModel = CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, Requirement.strFilePath.c_str());

		if (nullptr == pModel)
			return E_FAIL;

		if (FAILED(m_pProxy->Add_Prototype(Requirement.iPrototypeLevel, Requirement.strPrototypeTag, pModel)))
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