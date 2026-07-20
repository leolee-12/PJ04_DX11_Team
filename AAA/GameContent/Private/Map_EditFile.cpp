#include "Map_EditFile.h"
#include "Map_PresetCatalog.h"
#include "MapStage.h"
#include "MapSection.h"

#include "DataExporter.h"
#include "DataLoader.h"

#include <algorithm>
#include <filesystem>

NS_BEGIN(Client)

namespace
{
	using namespace std::filesystem;

	struct MAP_ENV_EDITED_DESC
	{
		_wstring strStableKey;

		_bool bHasRenderable = { false };
		_bool bRenderable = { true };

		_bool bHasEnableCulling = { false };
		_bool bEnableCulling = { true };

		_bool bHasUseCullDistance = { false };
		_bool bUseCullDistance = { true };

		_bool bHasUseCullFrustum = { false };
		_bool bUseCullFrustum = { true };

		_bool bHasShadow = { false }; // Edit JSON contains UseShadow.
		_bool bUseShadow = { false };

		_bool bHasWorldMatrix = { false };
		_float4x4 matWorld = {};

		_bool bHasCollMesh = { false }; // Edit JSON contains UseCollMesh.
		_bool bUseCollMesh = { false };

		_bool bHasNearDistAlpha = { false };
		_bool bUseNearDistAlpha = { false };
	};

	struct MAP_SECTION_EDITED_DESC
	{
		_wstring strStableKey;

		_bool bHasRenderable = { false };
		_bool bRenderable = { true };

		_bool bHasEnableCulling = { false };
		_bool bEnableCulling = { true };

		_bool bHasWorldMatrix = { false };
		_float4x4 matWorld = {};

		_bool bHasUseCollMesh = { false };
		_bool bUseCollMesh = { false };

		_bool bHasRenderID = { false };
		RENDERID eRenderID = { RENDERID::NONBLEND };
	};

	struct MAP_LD_EDITED_DESC
	{
		_wstring strStableKey;

		_bool bHasWorldMatrix = { false };
		_float4x4 matWorld = {};
	};

	HRESULT Resolve_EditFilePathFromManifest_Impl(const _wstring& strManifestPath, _wstring* pOutEditFilePath)
	{
		if (nullptr == pOutEditFilePath || strManifestPath.empty())
			return E_FAIL;

		pOutEditFilePath->clear();

		const path ManifestPath = path(strManifestPath).lexically_normal();
		if (ManifestPath.empty())
			return E_FAIL;

		const path EditFilePath = ManifestPath.parent_path()
			/ (ManifestPath.stem().wstring() + L".EditorOverride.json");

		*pOutEditFilePath = EditFilePath.lexically_normal().wstring();
		return S_OK;
	}

	HRESULT Resolve_PresetManifestPathForEditFile(_uint iPresetIndex, const _wstring& strManifestPath, _wstring* pOutManifestPath)
	{
		if (nullptr == pOutManifestPath)
			return E_FAIL;

		pOutManifestPath->clear();

		if (!strManifestPath.empty())
		{
			*pOutManifestPath = strManifestPath;
			return S_OK;
		}

		return CMap_PresetCatalog::Get_ManifestPath(iPresetIndex, pOutManifestPath);
	}

	json Save_Float4x4(const _float4x4& Mat)
	{
		json j = json::array();
		for (int iRow = 0; iRow < 4; ++iRow)
		{
			j.push_back(json::array({
					Mat.m[iRow][0],
					Mat.m[iRow][1],
					Mat.m[iRow][2],
					Mat.m[iRow][3]
				}));
		}
		return j;
	}

	HRESULT Load_Float4x4(const json& jValue, _float4x4* pOutMat)
	{
		if (nullptr == pOutMat)
			return E_FAIL;

		if (!jValue.is_array() || 4 != jValue.size())
			return E_FAIL;

		_float4x4 Mat{};
		for (int iRow = 0; iRow < 4; ++iRow)
		{
			const json& jRow = jValue[iRow];
			if (!jRow.is_array() || 4 != jRow.size())
				return E_FAIL;

			for (int iCol = 0; iCol < 4; ++iCol)
			{
				if (!jRow[iCol].is_number())
					return E_FAIL;

				Mat.m[iRow][iCol] = jRow[iCol].get<_float>();
			}
		}

		*pOutMat = Mat;
		return S_OK;
	}

	void Apply_WorldMatrixToGameObjectDesc(CGameObject::GAMEOBJECT_DESC* pOutDesc, const _float4x4& Mat)
	{
		if (nullptr == pOutDesc)
			return;

		pOutDesc->vRight	= _float4(Mat.m[0][0], Mat.m[0][1], Mat.m[0][2], Mat.m[0][3]);
		pOutDesc->vUp		= _float4(Mat.m[1][0], Mat.m[1][1], Mat.m[1][2], Mat.m[1][3]);
		pOutDesc->vLook		= _float4(Mat.m[2][0], Mat.m[2][1], Mat.m[2][2], Mat.m[2][3]);
		pOutDesc->vPosition	= _float4(Mat.m[3][0], Mat.m[3][1], Mat.m[3][2], Mat.m[3][3]);
	}

	void Apply_CommonOverrideToEnvDesc(ENV_OBJECT_DESC* pOutDesc, const EDIT_OBJECT_COMMON_OVERRIDE& Common)
	{
		if (nullptr == pOutDesc)
			return;

		const EDIT_OBJECT_POLICY& Policy = Common.Policy;

		if (Common.iPolicyMask & EDIT_CAP_RENDERABLE)
			pOutDesc->tCollision.bInvisibleCollision = !Policy.bRenderable;

		if (Common.iPolicyMask & EDIT_CAP_CULL_DISTANCE)
			pOutDesc->tRender.bUseCullDistance = Policy.bUseCullDistance;

		if (Common.iPolicyMask & EDIT_CAP_CULL_FRUSTUM)
			pOutDesc->tRender.bUseCullFrustum = Policy.bUseCullFrustum;

		if (Common.iPolicyMask & EDIT_CAP_SHADOW)
		{
			pOutDesc->tRender.bUseShadow = pOutDesc->tRender.bHasShadow && Policy.bUseShadow;
			pOutDesc->tRender.bShadowMappingCaster = pOutDesc->tRender.bUseShadow;
		}

		if (Common.bHasWorldMatrix)
		{
			pOutDesc->bHasWorldMatrix = true;
			pOutDesc->matWorld = Common.matWorld;

			CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(*pOutDesc);
			Apply_WorldMatrixToGameObjectDesc(&BaseDesc, Common.matWorld);
		}

		if (Common.iPolicyMask & EDIT_CAP_COLLISION_MESH)
			pOutDesc->tCollision.bUseCollMesh = pOutDesc->tCollision.bHasCollMesh && Policy.bUseCollMesh;
	}

	void Apply_CommonOverrideToMapSectionDesc(MAP_SECTION_DESC* pOutDesc, const EDIT_OBJECT_COMMON_OVERRIDE& Common)
	{
		if (nullptr == pOutDesc)
			return;

		const EDIT_OBJECT_POLICY& Policy = Common.Policy;

		if (Common.iPolicyMask & EDIT_CAP_RENDERABLE)
			pOutDesc->bRenderable = Policy.bRenderable;

		if (Common.iPolicyMask & EDIT_CAP_CULL_FRUSTUM)
			pOutDesc->bEnableCulling = Policy.bUseCullFrustum;

		if (Common.bHasWorldMatrix)
		{
			CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(*pOutDesc);
			Apply_WorldMatrixToGameObjectDesc(&BaseDesc, Common.matWorld);
		}

		if (Common.iPolicyMask & EDIT_CAP_COLLISION_MESH)
			pOutDesc->bUseCollMesh = Policy.bUseCollMesh;
	}

	void Apply_CommonOverrideToLDDesc(LD_OBJECT_DESC* pOutDesc, const EDIT_OBJECT_COMMON_OVERRIDE& Common)
	{
		if (nullptr == pOutDesc)
			return;

		if (Common.bHasWorldMatrix)
		{
			CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<CGameObject::GAMEOBJECT_DESC&>(*pOutDesc);
			Apply_WorldMatrixToGameObjectDesc(&BaseDesc, Common.matWorld);
		}

		if (Common.bHasPublishEvent)
			pOutDesc->strPublishEventTag = Common.strPublishEventTag;

		if (Common.bHasReceiveEvent)
			pOutDesc->strReceiveEventTag = Common.strReceiveEventTag;
	}

	void Apply_ClassOverrideToEnvDesc(ENV_OBJECT_DESC* pOutDesc, const EDIT_CLASS_OVERRIDE& ClassOverride)
	{
		if (nullptr == pOutDesc)
			return;

		const EDIT_ENVOBJECT_OVERRIDE* pEnvOverride = get_if<EDIT_ENVOBJECT_OVERRIDE>(&ClassOverride);
		if (nullptr == pEnvOverride)
			return;

		if (pEnvOverride->bHasNearDistAlpha)
			pOutDesc->tRender.bUseNearDistAlpha = pEnvOverride->bUseNearDistAlpha;
	}

	void Apply_ClassOverrideToMapSectionDesc(MAP_SECTION_DESC* pOutDesc, const EDIT_CLASS_OVERRIDE& ClassOverride)
	{
		UNREFERENCED_PARAMETER(pOutDesc);
		UNREFERENCED_PARAMETER(ClassOverride);
	}

	void Apply_ClassOverrideToLDEntry(LD_OBJECT_ENTRY* pOutEntry, const EDIT_CLASS_OVERRIDE& ClassOverride)
	{
		if (nullptr == pOutEntry)
			return;

		const EDIT_LD_WATER_OVERRIDE* pWaterOverride = get_if<EDIT_LD_WATER_OVERRIDE>(&ClassOverride);
		if (nullptr == pWaterOverride)
			return;

		LD_SURFACE_AREA_DESC* pSurfaceDesc = get_if<LD_SURFACE_AREA_DESC>(pOutEntry);
		if (nullptr == pSurfaceDesc)
			return;

		pSurfaceDesc->tWaterRenderDesc = pWaterOverride->RenderDesc;
	}

	void Apply_EditObjectOverrideToEnvDesc(ENV_OBJECT_DESC* pOutDesc, const EDIT_OBJECT_OVERRIDE_DESC& Edit)
	{
		if (nullptr == pOutDesc)
			return;

		if (EDITABLE_OBJECT_KIND::ENV_OBJECT != Edit.eKind)
			return;

		Apply_CommonOverrideToEnvDesc(pOutDesc, Edit.Common);
		Apply_ClassOverrideToEnvDesc(pOutDesc, Edit.ClassOverride);
	}

	void Apply_EditObjectOverrideToMapSectionDesc(MAP_SECTION_DESC* pOutDesc, const EDIT_OBJECT_OVERRIDE_DESC& Edit)
	{
		if (nullptr == pOutDesc)
			return;

		if (EDITABLE_OBJECT_KIND::MAP_SECTION != Edit.eKind)
			return;

		Apply_CommonOverrideToMapSectionDesc(pOutDesc, Edit.Common);
		Apply_ClassOverrideToMapSectionDesc(pOutDesc, Edit.ClassOverride);
	}

	void Apply_EditObjectOverrideToLDEntry(LD_OBJECT_ENTRY* pOutEntry, const EDIT_OBJECT_OVERRIDE_DESC& Edit)
	{
		if (nullptr == pOutEntry)
			return;

		if (EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT != Edit.eKind)
			return;

		LD_OBJECT_DESC& Desc = Get_LDObjectDesc(*pOutEntry);
		Apply_CommonOverrideToLDDesc(&Desc, Edit.Common);
		Apply_ClassOverrideToLDEntry(pOutEntry, Edit.ClassOverride);
	}

	void Set_CommonPolicyOverride(EDIT_OBJECT_COMMON_OVERRIDE* pCommon, _uint iCapability, _bool bValue)
	{
		if (nullptr == pCommon)
			return;

		pCommon->iPolicyMask |= iCapability;

		switch (iCapability)
		{
		case EDIT_CAP_RENDERABLE:
			pCommon->Policy.bRenderable = bValue;
			break;
		case EDIT_CAP_CULL_DISTANCE:
			pCommon->Policy.bUseCullDistance = bValue;
			break;
		case EDIT_CAP_CULL_FRUSTUM:
			pCommon->Policy.bUseCullFrustum = bValue;
			break;
		case EDIT_CAP_COLLISION_MESH:
			pCommon->Policy.bUseCollMesh = bValue;
			break;
		case EDIT_CAP_SHADOW:
			pCommon->Policy.bUseShadow = bValue;
			break;
		default:
			break;
		}
	}

	HRESULT Load_LegacyEnvEditedDesc(const json& jValue, MAP_ENV_EDITED_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return E_FAIL;

		*pOutDesc = {};

		if (!jValue.is_object())
			return E_FAIL;

		const auto IterRenderable = jValue.find("Renderable");
		if (IterRenderable != jValue.end())
		{
			if (!IterRenderable->is_boolean())
				return E_FAIL;

			pOutDesc->bHasRenderable = true;
			pOutDesc->bRenderable = IterRenderable->get<bool>();
		}

		const auto IterUseCullDistance = jValue.find("UseCullDistance");
		if (IterUseCullDistance != jValue.end())
		{
			if (!IterUseCullDistance->is_boolean())
				return E_FAIL;

			pOutDesc->bHasUseCullDistance = true;
			pOutDesc->bUseCullDistance = IterUseCullDistance->get<bool>();
		}

		const auto IterUseCullFrustum = jValue.find("UseCullFrustum");
		if (IterUseCullFrustum != jValue.end())
		{
			if (!IterUseCullFrustum->is_boolean())
				return E_FAIL;

			pOutDesc->bHasUseCullFrustum = true;
			pOutDesc->bUseCullFrustum = IterUseCullFrustum->get<bool>();
		}

		const auto IterUseShadow = jValue.find("UseShadow");
		if (IterUseShadow != jValue.end())
		{
			if (!IterUseShadow->is_boolean())
				return E_FAIL;

			pOutDesc->bHasShadow = true;
			pOutDesc->bUseShadow = IterUseShadow->get<bool>();
		}

		const auto IterWorldMatrix = jValue.find("WorldMatrix");
		if (IterWorldMatrix != jValue.end())
		{
			if (FAILED(Load_Float4x4(*IterWorldMatrix, &pOutDesc->matWorld)))
				return E_FAIL;

			pOutDesc->bHasWorldMatrix = true;
		}

		const auto IterUseCollMesh = jValue.find("UseCollMesh");
		if (IterUseCollMesh != jValue.end())
		{
			if (!IterUseCollMesh->is_boolean())
				return E_FAIL;

			pOutDesc->bHasCollMesh = true;
			pOutDesc->bUseCollMesh = IterUseCollMesh->get<bool>();
		}

		const auto IterNearDistAlpha = jValue.find("UseNearDistAlpha");
		if (IterNearDistAlpha != jValue.end())
		{
			if (!IterNearDistAlpha->is_boolean())
				return E_FAIL;

			pOutDesc->bHasNearDistAlpha = true;
			pOutDesc->bUseNearDistAlpha = IterNearDistAlpha->get<bool>();
		}

		return S_OK;
	}

	HRESULT Load_LegacyMapSectionEditedDesc(const json& jValue, MAP_SECTION_EDITED_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return E_FAIL;

		*pOutDesc = {};

		if (!jValue.is_object())
			return E_FAIL;

		const auto IterRenderable = jValue.find("Renderable");
		if (IterRenderable != jValue.end())
		{
			if (!IterRenderable->is_boolean())
				return E_FAIL;

			pOutDesc->bHasRenderable = true;
			pOutDesc->bRenderable = IterRenderable->get<bool>();
		}

		const auto IterEnableCulling = jValue.find("EnableCulling");
		if (IterEnableCulling != jValue.end())
		{
			if (!IterEnableCulling->is_boolean())
				return E_FAIL;

			pOutDesc->bHasEnableCulling = true;
			pOutDesc->bEnableCulling = IterEnableCulling->get<bool>();
		}

		const auto IterWorldMatrix = jValue.find("WorldMatrix");
		if (IterWorldMatrix != jValue.end())
		{
			if (FAILED(Load_Float4x4(*IterWorldMatrix, &pOutDesc->matWorld)))
				return E_FAIL;

			pOutDesc->bHasWorldMatrix = true;
		}

		const auto IterUseCollMesh = jValue.find("UseCollMesh");
		if (IterUseCollMesh != jValue.end())
		{
			if (!IterUseCollMesh->is_boolean())
				return E_FAIL;

			pOutDesc->bHasUseCollMesh = true;
			pOutDesc->bUseCollMesh = IterUseCollMesh->get<bool>();
		}

		return S_OK;
	}

	EDIT_OBJECT_OVERRIDE_DESC Convert_LegacyEnvEditToOverride(const MAP_ENV_EDITED_DESC& Edit)
	{
		EDIT_OBJECT_OVERRIDE_DESC Desc{};
		Desc.eKind = EDITABLE_OBJECT_KIND::ENV_OBJECT;
		Desc.strStableKey = Edit.strStableKey;

		if (Edit.bHasRenderable)		Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_RENDERABLE, Edit.bRenderable);
		if (Edit.bHasUseCullDistance)	Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_CULL_DISTANCE, Edit.bUseCullDistance);
		if (Edit.bHasUseCullFrustum)	Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_CULL_FRUSTUM, Edit.bUseCullFrustum);
		if (Edit.bHasCollMesh)			Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_COLLISION_MESH, Edit.bUseCollMesh);
		if (Edit.bHasShadow)			Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_SHADOW, Edit.bUseShadow);

		if (Edit.bHasWorldMatrix)
		{
			Desc.Common.bHasWorldMatrix = true;
			Desc.Common.matWorld = Edit.matWorld;
		}

		if (Edit.bHasNearDistAlpha)
		{
			EDIT_ENVOBJECT_OVERRIDE EnvOverride{};
			EnvOverride.bHasNearDistAlpha = true;
			EnvOverride.bUseNearDistAlpha = Edit.bUseNearDistAlpha;
			Desc.ClassOverride = EnvOverride;
		}

		return Desc;
	}

	EDIT_OBJECT_OVERRIDE_DESC Convert_LegacyMapSectionEditToOverride(const MAP_SECTION_EDITED_DESC& Edit)
	{
		EDIT_OBJECT_OVERRIDE_DESC Desc{};
		Desc.eKind = EDITABLE_OBJECT_KIND::MAP_SECTION;
		Desc.strStableKey = Edit.strStableKey;

		if (Edit.bHasRenderable)	Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_RENDERABLE, Edit.bRenderable);
		if (Edit.bHasEnableCulling)	Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_CULL_FRUSTUM, Edit.bEnableCulling);
		if (Edit.bHasUseCollMesh)	Set_CommonPolicyOverride(&Desc.Common, EDIT_CAP_COLLISION_MESH, Edit.bUseCollMesh);

		if (Edit.bHasWorldMatrix)
		{
			Desc.Common.bHasWorldMatrix = true;
			Desc.Common.matWorld = Edit.matWorld;
		}

		return Desc;
	}

	EDIT_OBJECT_OVERRIDE_DESC Convert_LegacyLDEditToOverride(const MAP_LD_EDITED_DESC& Edit)
	{
		EDIT_OBJECT_OVERRIDE_DESC Desc{};
		Desc.eKind = EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT;
		Desc.strStableKey = Edit.strStableKey;

		if (Edit.bHasWorldMatrix)
		{
			Desc.Common.bHasWorldMatrix = true;
			Desc.Common.matWorld = Edit.matWorld;
		}

		return Desc;
	}

	json Save_CommonOverride(const EDIT_OBJECT_COMMON_OVERRIDE& Common)
	{
		json jCommon = json::object();

		if (0u != Common.iPolicyMask)
		{
			json jPolicy = json::object();

			if (Common.iPolicyMask & EDIT_CAP_RENDERABLE)		jPolicy["Renderable"] = static_cast<bool>(Common.Policy.bRenderable);
			if (Common.iPolicyMask & EDIT_CAP_CULL_DISTANCE)	jPolicy["UseCullDistance"] = static_cast<bool>(Common.Policy.bUseCullDistance);
			if (Common.iPolicyMask & EDIT_CAP_CULL_FRUSTUM)		jPolicy["UseCullFrustum"] = static_cast<bool>(Common.Policy.bUseCullFrustum);
			if (Common.iPolicyMask & EDIT_CAP_COLLISION_MESH)	jPolicy["UseCollMesh"] = static_cast<bool>(Common.Policy.bUseCollMesh);
			if (Common.iPolicyMask & EDIT_CAP_SHADOW)			jPolicy["UseShadow"] = static_cast<bool>(Common.Policy.bUseShadow);

			if (!jPolicy.empty())	jCommon["Policy"] = jPolicy;
		}

		if (Common.bHasWorldMatrix)	jCommon["WorldMatrix"] = Save_Float4x4(Common.matWorld);
		if (Common.bHasPublishEvent)  jCommon["PublishEvent"] = WstrToStr(Common.strPublishEventTag);
		if (Common.bHasReceiveEvent)  jCommon["ReceiveEvent"] = WstrToStr(Common.strReceiveEventTag);

		return jCommon;
	}

	HRESULT Load_CommonPolicyField(const json& jPolicy, const char* pFieldName, EDIT_OBJECT_COMMON_OVERRIDE* pOutCommon, _uint iCapability)
	{
		const auto Iter = jPolicy.find(pFieldName);
		if (Iter == jPolicy.end())
			return S_OK;

		if (!Iter->is_boolean())
			return E_FAIL;

		Set_CommonPolicyOverride(pOutCommon, iCapability, Iter->get<bool>());
		return S_OK;
	}

	HRESULT Load_CommonOverride(const json& jCommon, EDIT_OBJECT_COMMON_OVERRIDE* pOutCommon)
	{
		if (nullptr == pOutCommon)
			return E_FAIL;

		*pOutCommon = {};

		if (!jCommon.is_object())
			return E_FAIL;

		const auto IterPolicy = jCommon.find("Policy");
		if (IterPolicy != jCommon.end())
		{
			if (!IterPolicy->is_object())
				return E_FAIL;

			if (FAILED(Load_CommonPolicyField(*IterPolicy, "Renderable", pOutCommon, EDIT_CAP_RENDERABLE)))			return E_FAIL;
			if (FAILED(Load_CommonPolicyField(*IterPolicy, "UseCullDistance", pOutCommon, EDIT_CAP_CULL_DISTANCE)))	return E_FAIL;
			if (FAILED(Load_CommonPolicyField(*IterPolicy, "UseCullFrustum", pOutCommon, EDIT_CAP_CULL_FRUSTUM)))	return E_FAIL;
			if (FAILED(Load_CommonPolicyField(*IterPolicy, "UseCollMesh", pOutCommon, EDIT_CAP_COLLISION_MESH)))	return E_FAIL;
			if (FAILED(Load_CommonPolicyField(*IterPolicy, "UseShadow", pOutCommon, EDIT_CAP_SHADOW)))				return E_FAIL;
		}

		const auto IterWorldMatrix = jCommon.find("WorldMatrix");
		if (IterWorldMatrix != jCommon.end())
		{
			if (FAILED(Load_Float4x4(*IterWorldMatrix, &pOutCommon->matWorld)))
				return E_FAIL;

			pOutCommon->bHasWorldMatrix = true;
		}
		const auto IterPublishEvent = jCommon.find("PublishEvent");
		if (IterPublishEvent != jCommon.end())
		{
			if (!IterPublishEvent->is_string())
				return E_FAIL;

			pOutCommon->bHasPublishEvent = true;
			pOutCommon->strPublishEventTag = StrToWstr(IterPublishEvent->get<string>());
		}

		const auto IterReceiveEvent = jCommon.find("ReceiveEvent");
		if (IterReceiveEvent != jCommon.end())
		{
			if (!IterReceiveEvent->is_string())
				return E_FAIL;

			pOutCommon->bHasReceiveEvent = true;
			pOutCommon->strReceiveEventTag = StrToWstr(IterReceiveEvent->get<string>());
		}
		return S_OK;
	}

	const char* Get_ClassOverrideName(const EDIT_CLASS_OVERRIDE& ClassOverride)
	{
		if (holds_alternative<EDIT_ENVOBJECT_OVERRIDE>(ClassOverride))		return "EnvObject";
		if (holds_alternative<EDIT_MAPSECTION_OVERRIDE>(ClassOverride))		return "MapSection";
		if (holds_alternative<EDIT_LEVELDESIGN_OVERRIDE>(ClassOverride))	return "LevelDesignObject";

		return "";
	}

	json Save_ClassOverride(const EDIT_CLASS_OVERRIDE& ClassOverride)
	{
		json jClassOverride = json::object();

		if (const EDIT_ENVOBJECT_OVERRIDE* pEnvOverride = get_if<EDIT_ENVOBJECT_OVERRIDE>(&ClassOverride))
		{
			if (pEnvOverride->bHasNearDistAlpha)	jClassOverride["UseNearDistAlpha"] = static_cast<bool>(pEnvOverride->bUseNearDistAlpha);
			if (pEnvOverride->bHasDecalAlpha)		jClassOverride["DecalAlpha"] = pEnvOverride->fDecalAlpha;
		}

		return jClassOverride;
	}

	HRESULT Load_ClassOverride(const string& strClassName, const json& jClassOverride, EDIT_CLASS_OVERRIDE* pOutClassOverride)
	{
		if (nullptr == pOutClassOverride)
			return E_FAIL;

		*pOutClassOverride = monostate{};

		if (!jClassOverride.is_object())
			return E_FAIL;

		if ("EnvObject" == strClassName)
		{
			EDIT_ENVOBJECT_OVERRIDE EnvOverride{};

			const auto IterNearDistAlpha = jClassOverride.find("UseNearDistAlpha");
			if (IterNearDistAlpha != jClassOverride.end())
			{
				if (!IterNearDistAlpha->is_boolean())
					return E_FAIL;

				EnvOverride.bHasNearDistAlpha = true;
				EnvOverride.bUseNearDistAlpha = IterNearDistAlpha->get<bool>();
			}

			const auto IterDecalAlpha = jClassOverride.find("DecalAlpha");
			if (IterDecalAlpha != jClassOverride.end())
			{
				if (!IterDecalAlpha->is_number())
					return E_FAIL;

				EnvOverride.bHasDecalAlpha = true;
				EnvOverride.fDecalAlpha = IterDecalAlpha->get<_float>();
			}

			*pOutClassOverride = EnvOverride;
			return S_OK;
		}

		if ("Bush" == strClassName)
		{
			// Legacy GenerateItem override: keep old edit files loadable, but discard the removed value.
			return S_OK;
		}

		if ("MapSection" == strClassName)
		{
			*pOutClassOverride = EDIT_MAPSECTION_OVERRIDE{};
			return S_OK;
		}

		if ("LevelDesignObject" == strClassName)
		{
			*pOutClassOverride = EDIT_LEVELDESIGN_OVERRIDE{};
			return S_OK;
		}

		return E_FAIL;
	}

	const char* Get_ObjectKindName(EDITABLE_OBJECT_KIND eKind)
	{
		switch (eKind)
		{
		case EDITABLE_OBJECT_KIND::MAP_SECTION:
			return "MapSection";
		case EDITABLE_OBJECT_KIND::ENV_OBJECT:
			return "EnvObject";
		case EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT:
			return "LevelDesignObject";
		default:
			return "MapSection";
		}
	}

	HRESULT Load_ObjectKindName(const string& strKind, EDITABLE_OBJECT_KIND* pOutKind)
	{
		if (nullptr == pOutKind)
			return E_FAIL;

		if ("MapSection" == strKind)
		{
			*pOutKind = EDITABLE_OBJECT_KIND::MAP_SECTION;
			return S_OK;
		}

		if ("EnvObject" == strKind)
		{
			*pOutKind = EDITABLE_OBJECT_KIND::ENV_OBJECT;
			return S_OK;
		}

		if ("LevelDesignObject" == strKind)
		{
			*pOutKind = EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT;
			return S_OK;
		}

		return E_FAIL;
	}

	json Save_NewEditObjectOverride(const EDIT_OBJECT_OVERRIDE_DESC& Edit)
	{
		json jEdit = json::object();
		jEdit["Kind"] = Get_ObjectKindName(Edit.eKind);

		if (0u != Edit.Common.iPolicyMask
			|| Edit.Common.bHasWorldMatrix
			|| Edit.Common.bHasPublishEvent
			|| Edit.Common.bHasReceiveEvent)
		{
			const json jCommon = Save_CommonOverride(Edit.Common);
			if (!jCommon.empty())
				jEdit["Common"] = jCommon;
		}

		if (Has_AnyClassOverride(Edit.ClassOverride))
		{
			const char* pClassName = Get_ClassOverrideName(Edit.ClassOverride);
			const json jClassOverride = Save_ClassOverride(Edit.ClassOverride);

			if (nullptr != pClassName && '\0' != pClassName[0] && !jClassOverride.empty())
			{
				jEdit["Class"] = pClassName;
				jEdit["ClassOverride"] = jClassOverride;
			}
		}

		return jEdit;
	}

	HRESULT Load_NewEditObjectOverride(const json& jValue, EDITABLE_OBJECT_KIND eFallbackKind, EDIT_OBJECT_OVERRIDE_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return E_FAIL;

		*pOutDesc = {};

		if (!jValue.is_object())
			return E_FAIL;

		EDIT_OBJECT_OVERRIDE_DESC Desc{};
		Desc.eKind = eFallbackKind;

		const auto IterKind = jValue.find("Kind");
		if (IterKind != jValue.end())
		{
			if (!IterKind->is_string())
				return E_FAIL;

			if (FAILED(Load_ObjectKindName(IterKind->get<string>(), &Desc.eKind)))
				return E_FAIL;
		}

		const auto IterCommon = jValue.find("Common");
		if (IterCommon != jValue.end())
		{
			if (FAILED(Load_CommonOverride(*IterCommon, &Desc.Common)))
				return E_FAIL;
		}

		const auto IterClass = jValue.find("Class");
		const auto IterClassOverride = jValue.find("ClassOverride");

		if (IterClassOverride != jValue.end())
		{
			if (IterClass == jValue.end() || !IterClass->is_string())
				return E_FAIL;

			if (FAILED(Load_ClassOverride(IterClass->get<string>(), *IterClassOverride, &Desc.ClassOverride)))
				return E_FAIL;
		}
		else if (IterClass != jValue.end() && !IterClass->is_string())
		{
			return E_FAIL;
		}

		*pOutDesc = Desc;
		return S_OK;
	}

	HRESULT Load_LegacyEditObjectOverride(const json& jValue, EDITABLE_OBJECT_KIND eFallbackKind, EDIT_OBJECT_OVERRIDE_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return E_FAIL;

		*pOutDesc = {};

		if (!jValue.is_object())
			return E_FAIL;

		switch (eFallbackKind)
		{
		case EDITABLE_OBJECT_KIND::ENV_OBJECT:
		{
			MAP_ENV_EDITED_DESC LegacyEdit{};
			if (FAILED(Load_LegacyEnvEditedDesc(jValue, &LegacyEdit)))
				return E_FAIL;

			*pOutDesc = Convert_LegacyEnvEditToOverride(LegacyEdit);
			return S_OK;
		}

		case EDITABLE_OBJECT_KIND::MAP_SECTION:
		{
			MAP_SECTION_EDITED_DESC LegacyEdit{};
			if (FAILED(Load_LegacyMapSectionEditedDesc(jValue, &LegacyEdit)))
				return E_FAIL;

			*pOutDesc = Convert_LegacyMapSectionEditToOverride(LegacyEdit);
			return S_OK;
		}
		case EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT:
		{
			MAP_LD_EDITED_DESC LegacyEdit{};

			const auto IterWorldMatrix = jValue.find("WorldMatrix");
			if (IterWorldMatrix != jValue.end())
			{
				if (FAILED(Load_Float4x4(*IterWorldMatrix, &LegacyEdit.matWorld)))
					return E_FAIL;

				LegacyEdit.bHasWorldMatrix = true;
			}

			*pOutDesc = Convert_LegacyLDEditToOverride(LegacyEdit);
			return S_OK;
		}
		default:
			return E_FAIL;
		}
	}

	HRESULT Load_EditObjectOverride(const json& jValue, EDITABLE_OBJECT_KIND eFallbackKind, EDIT_OBJECT_OVERRIDE_DESC* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return E_FAIL;

		if (!jValue.is_object())
			return E_FAIL;

		const bool bNewFormat =
			jValue.contains("Common")
			|| jValue.contains("Class")
			|| jValue.contains("ClassOverride")
			|| jValue.contains("Kind");

		if (bNewFormat)
			return Load_NewEditObjectOverride(jValue, eFallbackKind, pOutDesc);

		return Load_LegacyEditObjectOverride(jValue, eFallbackKind, pOutDesc);
	}

	json Save_EditObjectOverrideMap(const unordered_map<_wstring, EDIT_OBJECT_OVERRIDE_DESC>& EditedMap)
	{
		vector<_wstring> Keys;
		for (const auto& Pair : EditedMap)
		{
			if (!Pair.first.empty() && Has_AnyEdit(Pair.second))
				Keys.push_back(Pair.first);
		}

		sort(Keys.begin(), Keys.end());

		json jResult = json::object();
		for (const auto& strKey : Keys)
		{
			const auto Iter = EditedMap.find(strKey);
			if (Iter == EditedMap.end())
				continue;

			jResult[WstrToStr(strKey)] = Save_NewEditObjectOverride(Iter->second);
		}

		return jResult;
	}

	HRESULT Load_EditObjectOverrideMap(const json& jRoot, const char* pFieldName, unordered_map<_wstring, EDIT_OBJECT_OVERRIDE_DESC>* pOutMap, EDITABLE_OBJECT_KIND eFallbackKind)
	{
		if (nullptr == pFieldName || nullptr == pOutMap)
			return E_FAIL;

		pOutMap->clear();

		const auto IterField = jRoot.find(pFieldName);
		if (IterField == jRoot.end())
			return S_OK;

		if (!IterField->is_object())
			return E_FAIL;

		for (auto Iter = IterField->begin(); Iter != IterField->end(); ++Iter)
		{
			const _wstring strKey = StrToWstr(Iter.key());
			if (strKey.empty())
				continue;

			if (!Iter.value().is_object())
				return E_FAIL;

			EDIT_OBJECT_OVERRIDE_DESC Edit{};
			if (FAILED(Load_EditObjectOverride(Iter.value(), eFallbackKind, &Edit)))
				return E_FAIL;

			if (Edit.eKind != eFallbackKind)
				return E_FAIL;

			if (!Has_AnyEdit(Edit))
				continue;

			Edit.strStableKey = strKey;
			(*pOutMap)[strKey] = Edit;
		}

		return S_OK;
	}
}

_wstring CMap_EditFile::Make_EnvKey(const ENV_OBJECT_DESC& Desc)
{
	return Desc.wstrSourceFile + L"|"
		+ Desc.wstrSection + L"|"
		+ Desc.wstrEntryKey + L"|"
		+ to_wstring(Desc.iUid);
}

_wstring CMap_EditFile::Make_SectionKey(const _wstring& strStageName, const _wstring& strSectionName)
{
	if (strSectionName.empty())
		return L"";

	if (strStageName.empty())
		return strSectionName;

	return strStageName + L"|" + strSectionName;
}

_wstring CMap_EditFile::Make_SectionKey(const MAP_STAGE_DESC& StageDesc, const MAP_SECTION_DESC& SectionDesc)
{
	return Make_SectionKey(StageDesc.strStageName, SectionDesc.strSectionName);
}

_wstring CMap_EditFile::Make_LevelDesignKey(const LD_OBJECT_DESC& Desc)
{
	return Desc.strSourceFile + L"|"
		+ Desc.strSection + L"|"
		+ Desc.strEntryKey + L"|"
		+ to_wstring(Desc.iUid);
}

HRESULT CMap_EditFile::Apply_Change(MAP_PACKAGE* pInOutPackage, const MAP_EDIT_CHANGE& OverrideDesc)
{
	return Apply_Change(pInOutPackage, OverrideDesc, MAP_EDIT_APPLY_OPTIONS{});
}

HRESULT CMap_EditFile::Apply_Change(MAP_PACKAGE* pInOutPackage, const MAP_EDIT_CHANGE& OverrideDesc, const MAP_EDIT_APPLY_OPTIONS& Options)
{
	if (nullptr == pInOutPackage)
		return E_FAIL;

	if (Options.bApplyStage && !OverrideDesc.EditedMapSections.empty())
	{
		MAP_STAGE_DESC& StageDesc = pInOutPackage->StageDesc;
		for (MAP_SECTION_DESC& SectionDesc : StageDesc.SectionDescs)
		{
			const _wstring strKey = Make_SectionKey(StageDesc, SectionDesc);
			const auto Iter = OverrideDesc.EditedMapSections.find(strKey);
			if (Iter == OverrideDesc.EditedMapSections.end())
				continue;

			Apply_EditObjectOverrideToMapSectionDesc(&SectionDesc, Iter->second);
		}
	}

	if (Options.bApplyEnv && !OverrideDesc.EditedEnvObjects.empty())
	{
		for (ENV_OBJECT_DESC& Desc : pInOutPackage->EnvObjectDescs)
		{
			const _wstring strKey = Make_EnvKey(Desc);
			const auto Iter = OverrideDesc.EditedEnvObjects.find(strKey);
			if (Iter == OverrideDesc.EditedEnvObjects.end())
				continue;

			Apply_EditObjectOverrideToEnvDesc(&Desc, Iter->second);
		}
	}

	if (Options.bApplyEnv && !OverrideDesc.DeletedEnvObjectKeys.empty())
	{
		auto& EnvObjectDescs = pInOutPackage->EnvObjectDescs;
		EnvObjectDescs.erase(
			remove_if(
				EnvObjectDescs.begin(),
				EnvObjectDescs.end(),
				[&](const ENV_OBJECT_DESC& Desc)
				{
					const _wstring strKey = Make_EnvKey(Desc);
					return OverrideDesc.DeletedEnvObjectKeys.find(strKey)
						!= OverrideDesc.DeletedEnvObjectKeys.end();
				}),
			EnvObjectDescs.end());
	}

	if (Options.bApplyAddedObjects && !OverrideDesc.AddedMapObjects.empty())
	{
		pInOutPackage->AddedObjectDescs.insert(
			pInOutPackage->AddedObjectDescs.end(),
			OverrideDesc.AddedMapObjects.begin(),
			OverrideDesc.AddedMapObjects.end());
	}

	return S_OK;
}

HRESULT CMap_EditFile::Apply_LevelDesignChange(LD_PACKAGE* pInOutPackage, const MAP_EDIT_CHANGE& OverrideDesc)
{
	if (nullptr == pInOutPackage)
		return E_FAIL;

	if (!OverrideDesc.DeletedLevelDesignObjectKeys.empty())
	{
		auto& ObjectDescs = pInOutPackage->ObjectDescs;
		ObjectDescs.erase(
			remove_if(
				ObjectDescs.begin(),
				ObjectDescs.end(),
				[&](const LD_OBJECT_ENTRY& Entry)
				{
					const LD_OBJECT_DESC& Desc = Get_LDObjectDesc(Entry);
					const _wstring strKey = Make_LevelDesignKey(Desc);
					return OverrideDesc.DeletedLevelDesignObjectKeys.find(strKey)
						!= OverrideDesc.DeletedLevelDesignObjectKeys.end();
				}),
			ObjectDescs.end());
	}

	if (OverrideDesc.EditedLevelDesignObjects.empty())
		return S_OK;

	for (LD_OBJECT_ENTRY& Entry : pInOutPackage->ObjectDescs)
	{
		LD_OBJECT_DESC& Desc = Get_LDObjectDesc(Entry);
		const _wstring strKey = Make_LevelDesignKey(Desc);
		const auto Iter = OverrideDesc.EditedLevelDesignObjects.find(strKey);
		if (Iter == OverrideDesc.EditedLevelDesignObjects.end())
			continue;

		Apply_EditObjectOverrideToLDEntry(&Entry, Iter->second);
	}

	return S_OK;
}

HRESULT CMap_EditFile::Get_EditFilePath(const _wstring& strManifestPath, _wstring* pOutEditFilePath)
{
	return Resolve_EditFilePathFromManifest_Impl(strManifestPath, pOutEditFilePath);
}

HRESULT CMap_EditFile::Get_PresetEditFilePath(_uint iPresetIndex, const _wstring& strManifestPath, _wstring* pOutEditFilePath)
{
	_wstring strResolvedManifestPath;
	if (FAILED(Resolve_PresetManifestPathForEditFile(
		iPresetIndex,
		strManifestPath,
		&strResolvedManifestPath)))
	{
		return E_FAIL;
	}

	return Get_EditFilePath(strResolvedManifestPath, pOutEditFilePath);
}

HRESULT CMap_EditFile::Load_EditFile(
	const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	if (nullptr == pInOutMapContentDesc)
		return E_FAIL;

	if (nullptr != pOutMapStageOverride)
		*pOutMapStageOverride = json::object();

	if (nullptr != pOutHasMapStageOverride)
		*pOutHasMapStageOverride = false;

	if (strManifestPath.empty())
		return E_FAIL;

	if (pInOutMapContentDesc->strManifestPath.empty())
		pInOutMapContentDesc->strManifestPath = strManifestPath;

	_wstring strEditFilePath;
	if (FAILED(Get_EditFilePath(strManifestPath, &strEditFilePath)))
		return E_FAIL;

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strEditFilePath.c_str(), &strContent)))
		return S_FALSE;

	try
	{
		const json jRoot = json::parse(strContent);

		MAP_EDIT_DATA SavedMapContentDesc{};
		if (FAILED(Load_Data(jRoot, &SavedMapContentDesc)))
			return E_FAIL;

		if (SavedMapContentDesc.bHasMapContent)
		{
			pInOutMapContentDesc->bHasMapContent = true;
			pInOutMapContentDesc->Version = SavedMapContentDesc.Version;

			if (0 <= SavedMapContentDesc.iPresetIndex)
				pInOutMapContentDesc->iPresetIndex = SavedMapContentDesc.iPresetIndex;

			if (!SavedMapContentDesc.strManifestPath.empty())
				pInOutMapContentDesc->strManifestPath = SavedMapContentDesc.strManifestPath;

			pInOutMapContentDesc->bLoadStage = SavedMapContentDesc.bLoadStage;
			pInOutMapContentDesc->bLoadEnv = SavedMapContentDesc.bLoadEnv;
			pInOutMapContentDesc->OverrideDesc = SavedMapContentDesc.OverrideDesc;
		}

		if (nullptr != pOutMapStageOverride
			&& nullptr != pOutHasMapStageOverride
			&& jRoot.contains("MapStage")
			&& jRoot["MapStage"].is_object())
		{
			*pOutMapStageOverride = jRoot["MapStage"];
			*pOutHasMapStageOverride = true;
		}
	}
	catch (const json::exception&)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CMap_EditFile::Load_PresetEditFile(
	_uint iPresetIndex,
	const _wstring& strManifestPath,
	MAP_EDIT_DATA* pInOutMapContentDesc,
	json* pOutMapStageOverride,
	_bool* pOutHasMapStageOverride)
{
	if (nullptr == pInOutMapContentDesc)
		return E_FAIL;

	_wstring strResolvedManifestPath;
	if (FAILED(Resolve_PresetManifestPathForEditFile(
		iPresetIndex,
		strManifestPath,
		&strResolvedManifestPath)))
	{
		return E_FAIL;
	}

	if (pInOutMapContentDesc->strManifestPath.empty())
		pInOutMapContentDesc->strManifestPath = strResolvedManifestPath;

	const HRESULT hr = Load_EditFile(
		strResolvedManifestPath,
		pInOutMapContentDesc,
		pOutMapStageOverride,
		pOutHasMapStageOverride);

	pInOutMapContentDesc->iPresetIndex = static_cast<_int>(iPresetIndex);
	return hr;
}

HRESULT CMap_EditFile::Save_EditFile(
	const MAP_EDIT_DATA& MapContentDesc,
	const CMapStage* pStage)
{
	if (!MapContentDesc.bHasMapContent || MapContentDesc.strManifestPath.empty())
		return E_FAIL;

	_wstring strEditFilePath;
	if (FAILED(Get_EditFilePath(MapContentDesc.strManifestPath, &strEditFilePath)))
		return E_FAIL;

	json jRoot = json::object();
	jRoot["MapContent"] = Save_Data(MapContentDesc);

	if (nullptr != pStage)
		jRoot["MapStage"] = Save_Stage(pStage);

	return CDataExporter::Write_JsonFile(strEditFilePath.c_str(), jRoot);
}

HRESULT CMap_EditFile::Save_PresetEditFile(
	_uint iPresetIndex,
	const MAP_EDIT_DATA& MapContentDesc,
	const CMapStage* pStage)
{
	MAP_EDIT_DATA SaveDesc = MapContentDesc;
	SaveDesc.bHasMapContent = true;
	SaveDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

	if (SaveDesc.strManifestPath.empty())
	{
		if (FAILED(Resolve_PresetManifestPathForEditFile(
			iPresetIndex,
			L"",
			&SaveDesc.strManifestPath)))
		{
			return E_FAIL;
		}
	}

	return Save_EditFile(SaveDesc, pStage);
}

json CMap_EditFile::Save_Data(const MAP_EDIT_DATA& Desc)
{
	json jMapContent = json::object();

	jMapContent["Version"] = Desc.Version;

	if (0 <= Desc.iPresetIndex)
		jMapContent["PresetIndex"] = Desc.iPresetIndex;

	if (!Desc.strManifestPath.empty())
		jMapContent["Manifest"] = WstrToStr(Desc.strManifestPath);

	jMapContent["LoadStage"] = static_cast<bool>(Desc.bLoadStage);
	jMapContent["LoadEnv"] = static_cast<bool>(Desc.bLoadEnv);
	jMapContent["Override"] = Save_Change(Desc.OverrideDesc);

	return jMapContent;
}

HRESULT CMap_EditFile::Load_Data(const json& jLevelRoot, MAP_EDIT_DATA* pOutDesc)
{
	if (nullptr == pOutDesc)
		return E_FAIL;

	*pOutDesc = {};

	if (!jLevelRoot.is_object())
		return E_FAIL;

	const auto IterMapContent = jLevelRoot.find("MapContent");
	if (IterMapContent == jLevelRoot.end())
		return S_OK;

	if (!IterMapContent->is_object())
		return E_FAIL;

	pOutDesc->bHasMapContent = true;

	const json& jMapContent = *IterMapContent;

	const auto IterVersion = jMapContent.find("Version");
	if (IterVersion != jMapContent.end() && IterVersion->is_number_integer())
	{
		const int iVersion = IterVersion->get<int>();
		if (0 < iVersion)
			pOutDesc->Version = static_cast<_uint>(iVersion);
	}

	const auto IterPreset = jMapContent.find("PresetIndex");
	if (IterPreset != jMapContent.end() && IterPreset->is_number_integer())
		pOutDesc->iPresetIndex = IterPreset->get<int>();

	const auto IterManifest = jMapContent.find("Manifest");
	if (IterManifest != jMapContent.end() && IterManifest->is_string())
		pOutDesc->strManifestPath = StrToWstr(IterManifest->get<string>());

	const auto IterLoadStage = jMapContent.find("LoadStage");
	if (IterLoadStage != jMapContent.end() && IterLoadStage->is_boolean())
		pOutDesc->bLoadStage = IterLoadStage->get<bool>();

	const auto IterLoadEnv = jMapContent.find("LoadEnv");
	if (IterLoadEnv != jMapContent.end() && IterLoadEnv->is_boolean())
		pOutDesc->bLoadEnv = IterLoadEnv->get<bool>();

	const auto IterOverride = jMapContent.find("Override");
	if (IterOverride != jMapContent.end())
	{
		if (FAILED(Load_Change(*IterOverride, &pOutDesc->OverrideDesc)))
			return E_FAIL;
	}

	return S_OK;
}

json CMap_EditFile::Save_Change(const MAP_EDIT_CHANGE& Desc)
{
	json jOverride = json::object();
	jOverride["Version"] = Desc.Version < 6u ? 6u : Desc.Version;

	vector<_wstring> DeletedKeys(
		Desc.DeletedEnvObjectKeys.begin(),
		Desc.DeletedEnvObjectKeys.end());

	sort(DeletedKeys.begin(), DeletedKeys.end());

	json jDeletedEnvObjects = json::array();
	for (const auto& strKey : DeletedKeys)
		jDeletedEnvObjects.push_back(WstrToStr(strKey));

	vector<_wstring> DeletedLevelDesignKeys(
		Desc.DeletedLevelDesignObjectKeys.begin(),
		Desc.DeletedLevelDesignObjectKeys.end());

	sort(DeletedLevelDesignKeys.begin(), DeletedLevelDesignKeys.end());

	json jDeletedLevelDesignObjects = json::array();
	for (const auto& strKey : DeletedLevelDesignKeys)
		jDeletedLevelDesignObjects.push_back(WstrToStr(strKey));

	jOverride["DeletedEnvObjects"] = jDeletedEnvObjects;
	jOverride["DeletedLevelDesignObjects"] = jDeletedLevelDesignObjects;
	jOverride["EditedEnvObjects"] = Save_EditObjectOverrideMap(Desc.EditedEnvObjects);
	jOverride["EditedMapSections"] = Save_EditObjectOverrideMap(Desc.EditedMapSections);
	jOverride["EditedLevelDesignObjects"] = Save_EditObjectOverrideMap(Desc.EditedLevelDesignObjects);

	jOverride["AddedMapObjects"] = json::array();
	for (const auto& Added : Desc.AddedMapObjects)
	{
		json jAdded = json::object();
		jAdded["Prototype_Tag"] = WstrToStr(Added.strPrototypeTag);
		jAdded["Layer_Tag"] = WstrToStr(Added.strLayerTag);
		jAdded["Object_Tag"] = WstrToStr(Added.strObjectTag);

		if (!Added.strReplacedEnvKey.empty())
			jAdded["Replaced_Env_Key"] = WstrToStr(Added.strReplacedEnvKey);

		jAdded["Object"] = Added.jObject;
		jOverride["AddedMapObjects"].push_back(jAdded);
	}

	return jOverride;
}

HRESULT CMap_EditFile::Load_Change(const json& jOverride, MAP_EDIT_CHANGE* pOutDesc)
{
	if (nullptr == pOutDesc)
		return E_FAIL;

	*pOutDesc = {};

	if (!jOverride.is_object())
		return E_FAIL;

	const auto IterVersion = jOverride.find("Version");
	if (IterVersion != jOverride.end() && IterVersion->is_number_integer())
	{
		const int iVersion = IterVersion->get<int>();
		if (0 < iVersion)
			pOutDesc->Version = static_cast<_uint>(iVersion);
	}

	const auto IterDeleted = jOverride.find("DeletedEnvObjects");
	if (IterDeleted != jOverride.end())
	{
		if (!IterDeleted->is_array())
			return E_FAIL;

		for (const auto& jKey : *IterDeleted)
		{
			if (!jKey.is_string())
				continue;

			const _wstring strKey = StrToWstr(jKey.get<string>());
			if (!strKey.empty())
				pOutDesc->DeletedEnvObjectKeys.insert(strKey);
		}
	}

	const auto IterDeletedLevelDesign = jOverride.find("DeletedLevelDesignObjects");
	if (IterDeletedLevelDesign != jOverride.end())
	{
		if (!IterDeletedLevelDesign->is_array())
			return E_FAIL;

		for (const auto& jKey : *IterDeletedLevelDesign)
		{
			if (!jKey.is_string())
				continue;

			const _wstring strKey = StrToWstr(jKey.get<string>());
			if (!strKey.empty())
				pOutDesc->DeletedLevelDesignObjectKeys.insert(strKey);
		}
	}

	if (FAILED(Load_EditObjectOverrideMap(jOverride, "EditedEnvObjects", &pOutDesc->EditedEnvObjects, EDITABLE_OBJECT_KIND::ENV_OBJECT)))
		return E_FAIL;

	if (FAILED(Load_EditObjectOverrideMap(jOverride, "EditedMapSections", &pOutDesc->EditedMapSections, EDITABLE_OBJECT_KIND::MAP_SECTION)))
		return E_FAIL;

	if (FAILED(Load_EditObjectOverrideMap(jOverride, "EditedLevelDesignObjects", &pOutDesc->EditedLevelDesignObjects, EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT)))
		return E_FAIL;

	const auto IterAdded = jOverride.find("AddedMapObjects");
	if (IterAdded != jOverride.end())
	{
		if (!IterAdded->is_array())
			return E_FAIL;

		for (const auto& jAdded : *IterAdded)
		{
			if (!jAdded.is_object())
				continue;

			MAP_ADD_OBJECT AddedDesc{};
			AddedDesc.jObject = json::object();

			const auto IterPrototype = jAdded.find("Prototype_Tag");
			if (IterPrototype != jAdded.end() && IterPrototype->is_string())
				AddedDesc.strPrototypeTag = StrToWstr(IterPrototype->get<string>());

			const auto IterLayer = jAdded.find("Layer_Tag");
			if (IterLayer != jAdded.end() && IterLayer->is_string())
				AddedDesc.strLayerTag = StrToWstr(IterLayer->get<string>());

			const auto IterObjectTag = jAdded.find("Object_Tag");
			if (IterObjectTag != jAdded.end() && IterObjectTag->is_string())
				AddedDesc.strObjectTag = StrToWstr(IterObjectTag->get<string>());

			const auto IterReplacedEnvKey = jAdded.find("Replaced_Env_Key");
			if (IterReplacedEnvKey != jAdded.end() && IterReplacedEnvKey->is_string())
				AddedDesc.strReplacedEnvKey = StrToWstr(IterReplacedEnvKey->get<string>());

			const auto IterObject = jAdded.find("Object");
			if (IterObject != jAdded.end())
			{
				if (IterObject->is_object())
					AddedDesc.jObject = *IterObject;
				else if (!IterObject->is_null())
					continue;
			}

			if (AddedDesc.strPrototypeTag.empty()
				|| AddedDesc.strLayerTag.empty()
				|| AddedDesc.strObjectTag.empty())
			{
				continue;
			}

			pOutDesc->AddedMapObjects.push_back(AddedDesc);
		}
	}

	return S_OK;
}

json CMap_EditFile::Save_Stage(const CMapStage* pStage)
{
	if (nullptr == pStage)
		return json::object();

	json jOverride = json::object();
	jOverride["StageName"] = WstrToStr(pStage->Get_StageName());
	jOverride["Sections"] = json::array();

	const auto& Sections = pStage->Get_Sections();
	for (const CMapSection* pSection : Sections)
	{
		if (nullptr == pSection)
			continue;

		jOverride["Sections"].push_back(pSection->Serialize_SectionState());
	}

	return jOverride;
}

HRESULT CMap_EditFile::Apply_Stage(CMapStage* pStage, const json& jOverride)
{
	if (nullptr == pStage)
		return E_FAIL;

	if (jOverride.is_null())
		return S_OK;

	if (!jOverride.is_object())
		return E_FAIL;

	pStage->Deserialize(jOverride);
	return S_OK;
}

NS_END
