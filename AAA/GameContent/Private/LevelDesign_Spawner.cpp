#include "LevelDesign_Spawner.h"
#include "LevelDesign_Registry.h"
#include "LevelDesign_Rail.h"
#include "RailRideable.h"
#include "Deformable.h"

#include "GameInstance.h"

namespace
{
	struct PENDING_RAIL_BINDING
	{
		IRailRideable* pReceiver = nullptr;
		_uint iRailUid = 0u;
		_uint iNodeIndex = 0u;
	};

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
					|| is_same_v<T, LD_DEFORMOBJECT_DESC>
					|| is_same_v<T, LD_FOOD_DESC>
					|| is_same_v<T, LD_POINT_DESC>
					|| is_same_v<T, LD_BUSH_DESC>
					|| is_same_v<T, LD_SURFACE_AREA_DESC>)
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

	vector<PENDING_RAIL_BINDING> PendingRailBindings;
	vector<_uint> PendingRailVisuals;
	HRESULT hrFinal = S_OK;

	for (const LD_OBJECT_ENTRY& Desc : Package.ObjectDescs)
	{
		const LD_OBJECT_DESC& BaseDesc = Get_LDObjectDesc(Desc);

		CGameObject* pCreatedObject = nullptr;
		if (FAILED(Spawn_One(Desc, Request, pOutReport, &pCreatedObject)))
		{
			hrFinal = S_FALSE;
			continue;
		}

		if (0 == BaseDesc.iTargetRailUid || nullptr == pCreatedObject)
			continue;

		IDeformable* pDeformable = dynamic_cast<IDeformable*>(pCreatedObject);
		if (nullptr != pDeformable && DEFORM_TYPE::COASTER == pDeformable->Get_DeformType())
			PendingRailVisuals.push_back(BaseDesc.iTargetRailUid);

		IRailRideable* pReceiver = dynamic_cast<IRailRideable*>(pCreatedObject);
		if (nullptr != pReceiver)
			PendingRailBindings.push_back({ pReceiver, BaseDesc.iTargetRailUid, BaseDesc.iTargetRailNodeIndex });
	}

	for (const PENDING_RAIL_BINDING& Binding : PendingRailBindings)
	{
		const CLevelDesign_Rail* pRail = CLevelDesign_Rail::Find_ByUid(m_pProxy, Request.iPlaceLevel, Binding.iRailUid);
		if (nullptr == pRail)
		{
#ifdef _DEBUG
			const _wstring strMessage = L"[LevelDesign_Spawner] Rail not found: " + to_wstring(Binding.iRailUid) + L"\n";
			OutputDebugStringW(strMessage.c_str());
#endif
			continue;
		}

		RAIL_BIND_CONTEXT Context{};
		Context.pRailDesc = &pRail->Get_RailDesc();
		Context.pRailTrack = pRail->Get_RailTrack();
		Context.iRailUid = Binding.iRailUid;
		Context.iStartNodeIndex = Binding.iNodeIndex;

		if (FAILED(Binding.pReceiver->Bind_Rail(Context)))
		{
#ifdef _DEBUG
			const _wstring strMessage = L"[LevelDesign_Spawner] Rail bind failed: "
				+ to_wstring(Binding.iRailUid)
				+ L" NodeIndex=" + to_wstring(Binding.iNodeIndex)
				+ L"\n";
			OutputDebugStringW(strMessage.c_str());
#endif
		}
	}

	for (const _uint iRailUid : PendingRailVisuals)
	{
		CLevelDesign_Rail* pRail = CLevelDesign_Rail::Find_ByUid(m_pProxy, Request.iPlaceLevel, iRailUid);
		if (nullptr == pRail)
		{
#ifdef _DEBUG
			const _wstring strMessage = L"[LevelDesign_Spawner] Visual Rail not found: " + to_wstring(iRailUid) + L"\n";
			OutputDebugStringW(strMessage.c_str());
#endif
			continue;
		}

		if (FAILED(pRail->Enable_Visual(CLevelDesign_Rail::RAIL_VISUAL_TYPE::COASTER, Request.Levels.iModelPrototypeLevel)))
		{
#ifdef _DEBUG
			const _wstring strMessage = L"[LevelDesign_Spawner] Rail visual enable failed: " + to_wstring(iRailUid) + L"\n";
			OutputDebugStringW(strMessage.c_str());
#endif
		}
	}

	return hrFinal;
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
		if (nullptr != pInOutReport)
			++pInOutReport->iSkippedCreateFailedCount;

		return E_FAIL;
	}

	if (Resolved.bFallback && nullptr != pInOutReport)
		++pInOutReport->iFallbackSpecCount;

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
