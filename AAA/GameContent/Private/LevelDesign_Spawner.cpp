#include "LevelDesign_Spawner.h"
#include "LevelDesign_Registry.h"
#include "LevelDesign_Rail.h"
#include "RailDataReceiver.h"

#include "GameInstance.h"

namespace
{
	void Apply_ModelProtoLevel(LD_OBJECT_ENTRY* pEntry, _uint iModelProtoLevel)
	{
		if (nullptr == pEntry)
			return;

		std::visit([&](auto& Desc)
			{
				using T = decay_t<decltype(Desc)>;

				if constexpr (is_same_v<T, LD_BREAKABLE_DESC>
					|| is_same_v<T, LD_LADDER_DESC>
					|| is_same_v<T, LD_EVENTOBJECT_DESC>
					|| is_same_v<T, LD_FOOD_DESC>
					|| is_same_v<T, LD_POINT_DESC>
					|| is_same_v<T, LD_BUSH_DESC>)
				{
					Desc.iModelProtoLevel = iModelProtoLevel;
				}
			}, *pEntry);
	}
}

CLevelDesign_Spawner::CLevelDesign_Spawner()
	: m_pProxy{ CGameInstance::GetProxy() }
{
}

HRESULT CLevelDesign_Spawner::Spawn(const LD_PACKAGE& Package, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pOutReport)
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

	vector<pair<IRailDataReceiver*, _uint>> PendingRailBindings;

	for (const LD_OBJECT_ENTRY& Desc : Package.ObjectDescs)
	{
		const LD_OBJECT_DESC& BaseDesc = Get_LDObjectDesc(Desc);

		CGameObject* pCreatedObject = nullptr;
		if (FAILED(Spawn_One(Desc, Request, pOutReport, &pCreatedObject)))
			return E_FAIL;

		if (0 == BaseDesc.iTargetRailUid || nullptr == pCreatedObject)
			continue;

		IRailDataReceiver* pReceiver = dynamic_cast<IRailDataReceiver*>(pCreatedObject);
		if (nullptr != pReceiver)
			PendingRailBindings.emplace_back(pReceiver, BaseDesc.iTargetRailUid);
	}

	for (const auto& [pReceiver, iRailUid] : PendingRailBindings)
	{
		const CLevelDesign_Rail* pRail = CLevelDesign_Rail::Find_ByUid(m_pProxy, Request.iPlaceLevel, iRailUid);
		if (nullptr != pRail)
		{
			pReceiver->Set_RailDesc(pRail->Get_RailDesc());
			continue;
		}

#ifdef _DEBUG
		const _wstring strMessage = L"[LevelDesign_Spawner] Rail not found: " + to_wstring(iRailUid) + L"\n";
		OutputDebugStringW(strMessage.c_str());
#endif
	}

	return S_OK;
}

HRESULT CLevelDesign_Spawner::Spawn_One(const LD_OBJECT_ENTRY& Desc, const LD_SPAWN_REQUEST& Request, LD_LOAD_RESULT* pInOutReport, CGameObject** ppOutCreatedObject)
{
	if (nullptr == m_pProxy)
		return E_FAIL;

	if (nullptr != ppOutCreatedObject)
		*ppOutCreatedObject = nullptr;

	LD_RESOLVED_SPAWN Resolved{};
	if (!CLevelDesign_Registry::Resolve(Desc, &Resolved))
	{
		return E_FAIL;
	}

	const LD_SPAWN_SPEC& Spec = Resolved.Spec;
	Apply_ModelProtoLevel(&Resolved.ObjectDesc, Request.Levels.iModelPrototypeLevel);
	const LD_OBJECT_DESC& SpawnDesc = Get_LDObjectDesc(Resolved.ObjectDesc);
	const _wstring strObjectTag = Make_ObjectTag(SpawnDesc);

	CGameObject* pCreatedObject = nullptr;

	const HRESULT hr = m_pProxy->Add_GameObject_Return(
		&pCreatedObject,
		Request.Levels.iPrototypeLevel,
		Spec.strPrototypeTag,
		Request.iPlaceLevel,
		Spec.strLayerTag,
		strObjectTag,
		Resolved.Get_SpawnArgument());

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

	if (nullptr != ppOutCreatedObject)
		*ppOutCreatedObject = pCreatedObject;

	return S_OK;
}

_wstring CLevelDesign_Spawner::Make_ObjectTag(const LD_OBJECT_DESC& Desc) const
{
	if (Desc.iUid != 0)
		return Desc.strObjectName + L"_" + to_wstring(Desc.iUid);

	if (!Desc.strEntryKey.empty())
		return Desc.strObjectName + L"_" + Desc.strEntryKey;

	return Desc.strObjectName;
}

CLevelDesign_Spawner* CLevelDesign_Spawner::Create()
{
	return new CLevelDesign_Spawner();
}

void CLevelDesign_Spawner::Free()
{
	Safe_Release(m_pProxy);
	__super::Free();
}