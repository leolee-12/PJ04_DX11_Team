#pragma once
#include "Engine_Defines.h"
#include "Parsing_Utils.h"

NS_BEGIN(Engine)

inline json Save_MeshLayerBindEx(const MESH_LAYER_TEX_BIND_EX& Bind)
{
	json j = json::object();

	if (Bind.bEnable)		j["Enable"] = true;
	if (Bind.iTexType != 0)	j["TexType"] = Bind.iTexType;
	if (Bind.iSlot >= 0)	j["Slot"] = Bind.iSlot;
	if (Bind.iUVIndex != 0)	j["UVIndex"] = Bind.iUVIndex;
	if (Bind.vUVScale.x != 1.f || Bind.vUVScale.y != 1.f)	j["Scale"] = { Bind.vUVScale.x, Bind.vUVScale.y };
	if (Bind.vUVOffset.x != 0.f || Bind.vUVOffset.y != 0.f)	j["Offset"] = { Bind.vUVOffset.x, Bind.vUVOffset.y };
	if (Bind.fUVRotate != 0.f)	j["Rotate"] = Bind.fUVRotate;

	return j;
}

inline bool Has_MeshLayerBindExValue(const MESH_LAYER_TEX_BIND_EX& Bind)
{
	return Bind.bEnable
		|| Bind.iTexType != 0
		|| Bind.iSlot >= 0
		|| Bind.iUVIndex != 0
		|| Bind.vUVScale.x != 1.f
		|| Bind.vUVScale.y != 1.f
		|| Bind.vUVOffset.x != 0.f
		|| Bind.vUVOffset.y != 0.f
		|| Bind.fUVRotate != 0.f;
}

inline json Save_MeshLayerEx(const MESH_LAYER_IDX& Layer)
{
	json jLayerEx = json::object();

	jLayerEx["Use"] = static_cast<bool>(Layer.bUseLayerEx);

	json jGroups = json::array();

	for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
	{
		json jEntries = json::array();

		for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
		{
			const MESH_LAYER_TEX_BIND_EX& Bind = Layer.LayerEx[g][e];

			if (Has_MeshLayerBindExValue(Bind))
				jEntries.push_back(Save_MeshLayerBindEx(Bind));
			else
				jEntries.push_back(json::object());
		}

		jGroups.push_back(jEntries);
	}

	jLayerEx["Groups"] = jGroups;
	return jLayerEx;
}

inline bool Has_MeshLayerExValue(const MESH_LAYER_IDX& Layer)
{
	if (Layer.bUseLayerEx)
		return true;

	for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
	{
		for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
		{
			if (Has_MeshLayerBindExValue(Layer.LayerEx[g][e]))
				return true;
		}
	}

	return false;
}

inline void Load_MeshLayerBindEx(const json& j, MESH_LAYER_TEX_BIND_EX* pOut)
{
	if (nullptr == pOut || !j.is_object())
		return;

	MESH_LAYER_TEX_BIND_EX Bind{};

	if (j.contains("Enable") && j["Enable"].is_boolean())
		Bind.bEnable = j["Enable"].get<bool>();

	if (j.contains("TexType") && j["TexType"].is_number_integer())
	{
		const int iTexType = j["TexType"].get<int>();
		if (0 <= iTexType && iTexType < static_cast<int>(MTEX_TYPE_MAX))
			Bind.iTexType = static_cast<unsigned int>(iTexType);
	}

	if (j.contains("Slot") && j["Slot"].is_number_integer())
	{
		const int iSlot = j["Slot"].get<int>();
		Bind.iSlot = (iSlot >= 0) ? iSlot : -1;
	}

	if (j.contains("UVIndex") && j["UVIndex"].is_number_integer())
	{
		const int iUVIndex = j["UVIndex"].get<int>();
		if (0 <= iUVIndex && iUVIndex <= 3)
			Bind.iUVIndex = static_cast<_uint>(iUVIndex);
	}

	JsonUtils::Try_ReadFloat2Array(j, "Scale", &Bind.vUVScale);
	JsonUtils::Try_ReadFloat2Array(j, "Offset", &Bind.vUVOffset);

	if (j.contains("Rotate") && j["Rotate"].is_number())
		Bind.fUVRotate = j["Rotate"].get<_float>();

	*pOut = Bind;
}

inline void Load_MeshLayerEx(const json& jLayerEx, MESH_LAYER_IDX* pOutLayer)
{
	if (nullptr == pOutLayer || !jLayerEx.is_object())
		return;

	if (jLayerEx.contains("Use") && jLayerEx["Use"].is_boolean())
		pOutLayer->bUseLayerEx = jLayerEx["Use"].get<bool>();

	const auto IterGroups = jLayerEx.find("Groups");
	if (IterGroups == jLayerEx.end() || !IterGroups->is_array())
		return;

	const json& jGroups = *IterGroups;

	const _uint iGroupCount =
		min<_uint>(static_cast<_uint>(jGroups.size()), MESH_LAYER_EX_GROUP_COUNT);

	for (_uint g = 0; g < iGroupCount; ++g)
	{
		if (!jGroups[g].is_array())
			continue;

		const json& jEntries = jGroups[g];
		const _uint iEntryCount =
			min<_uint>(static_cast<_uint>(jEntries.size()), MESH_LAYER_EX_ENTRY_COUNT);

		for (_uint e = 0; e < iEntryCount; ++e)
			Load_MeshLayerBindEx(jEntries[e], &pOutLayer->LayerEx[g][e]);
	}
}

inline json Save_MeshLayer(const MESH_LAYER_IDX& Layer)
{
	json jMesh = json::object();

	if (Layer.iPass >= 0)			jMesh["Pass"] = Layer.iPass;
	if (Layer.iUVIndex != 0)		jMesh["UVIndex"] = Layer.iUVIndex;
	if (Layer.iUnknownUVIndex != 0)	jMesh["UnknownUVIndex"] = Layer.iUnknownUVIndex;

	if (Layer.iExtraUVIndex[0] != 0 || Layer.iExtraUVIndex[1] != 0 || Layer.iExtraUVIndex[2] != 0 || Layer.iExtraUVIndex[3] != 0)
		jMesh["ExtraUVIndex"] = { Layer.iExtraUVIndex[0], Layer.iExtraUVIndex[1], Layer.iExtraUVIndex[2], Layer.iExtraUVIndex[3] };

	if (Layer.iFlags != 0)	jMesh["Flags"] = Layer.iFlags;

	if (Layer.bUseUVTransform)
	{
		jMesh["UseUVTransform"] = true;
		jMesh["UVScale"] = { Layer.vUVScale.x, Layer.vUVScale.y };
		jMesh["UVScaleNormal"] = { Layer.vUVScaleNormal.x, Layer.vUVScaleNormal.y };
		jMesh["UVScaleMaterial"] = { Layer.vUVScaleMaterial.x, Layer.vUVScaleMaterial.y };
		jMesh["UVRotate"] = Layer.fUVRotate;
		jMesh["UVOffset"] = { Layer.vUVOffset.x, Layer.vUVOffset.y };
	}

	if (Layer.fNormalStrength != 1.f)	jMesh["NormalStrength"] = Layer.fNormalStrength;
	if (Layer.fMaskStrength != 1.f)		jMesh["MaskStrength"] = Layer.fMaskStrength;

	if (Layer.vRenderColor.x != 1.f || Layer.vRenderColor.y != 1.f || Layer.vRenderColor.z != 1.f || Layer.vRenderColor.w != 1.f)
		jMesh["RenderColor"] = { Layer.vRenderColor.x, Layer.vRenderColor.y, Layer.vRenderColor.z, Layer.vRenderColor.w };

	if (Layer.vEmissiveColor.x != 0.f || Layer.vEmissiveColor.y != 0.f || Layer.vEmissiveColor.z != 0.f || Layer.vEmissiveColor.w != 0.f)
		jMesh["EmissiveColor"] = { Layer.vEmissiveColor.x, Layer.vEmissiveColor.y, Layer.vEmissiveColor.z, Layer.vEmissiveColor.w };

	if (Layer.vMRA.x != 0.f || Layer.vMRA.y != 1.f || Layer.vMRA.z != 1.f)
		jMesh["MRAValue"] = { Layer.vMRA.x, Layer.vMRA.y, Layer.vMRA.z };

	const unsigned int iUnknown = static_cast<unsigned int>(MTEX_TYPE::UNKNOWN);
	if (Layer.iExtraTexType[0] != iUnknown || Layer.iExtraTexType[1] != iUnknown || Layer.iExtraTexType[2] != iUnknown || Layer.iExtraTexType[3] != iUnknown)
		jMesh["ExtraTexType"] = { Layer.iExtraTexType[0], Layer.iExtraTexType[1], Layer.iExtraTexType[2], Layer.iExtraTexType[3] };

	if (Layer.iExtraBind[0] >= 0 || Layer.iExtraBind[1] >= 0 || Layer.iExtraBind[2] >= 0 || Layer.iExtraBind[3] >= 0)
		jMesh["ExtraBind"] = { Layer.iExtraBind[0], Layer.iExtraBind[1], Layer.iExtraBind[2], Layer.iExtraBind[3] };

	for (_uint iTexType = 0; iTexType < MTEX_TYPE_MAX; ++iTexType)
	{
		if (Layer.idx[iTexType] != 0)
			jMesh[to_string(iTexType)] = Layer.idx[iTexType];
	}

	if (Has_MeshLayerExValue(Layer))
		jMesh["LayerEx"] = Save_MeshLayerEx(Layer);

	return jMesh;
}

inline HRESULT Load_MeshLayer(const json& jMesh, MESH_LAYER_IDX* pOutLayer)
{
	if (nullptr == pOutLayer || !jMesh.is_object())
		return E_FAIL;

	MESH_LAYER_IDX Layer{};

	if (jMesh.contains("Pass") && jMesh["Pass"].is_number_integer())
		Layer.iPass = jMesh["Pass"].get<int>();

	if (jMesh.contains("UVIndex") && jMesh["UVIndex"].is_number_integer())
	{
		const int iValue = jMesh["UVIndex"].get<int>();
		if (0 <= iValue && iValue <= 3)
			Layer.iUVIndex = static_cast<_uint>(iValue);
	}

	if (jMesh.contains("UnknownUVIndex") && jMesh["UnknownUVIndex"].is_number_integer())
	{
		const int iValue = jMesh["UnknownUVIndex"].get<int>();
		if (0 <= iValue && iValue <= 3)
			Layer.iUnknownUVIndex = static_cast<_uint>(iValue);
	}

	if (jMesh.contains("ExtraUVIndex") && jMesh["ExtraUVIndex"].is_array() && jMesh["ExtraUVIndex"].size() == 4)
	{
		for (_uint i = 0; i < 4; ++i)
		{
			if (!jMesh["ExtraUVIndex"][i].is_number_integer())
				continue;

			const int iValue = jMesh["ExtraUVIndex"][i].get<int>();
			if (0 <= iValue && iValue <= 3)
				Layer.iExtraUVIndex[i] = static_cast<_uint>(iValue);
		}
	}

	if (jMesh.contains("Flags") && jMesh["Flags"].is_number_integer())
	{
		const int iFlags = jMesh["Flags"].get<int>();
		if (0 <= iFlags)
			Layer.iFlags = static_cast<_uint>(iFlags);
	}

	if (jMesh.contains("RenderColor") && jMesh["RenderColor"].is_array() && jMesh["RenderColor"].size() == 4)
	{
		if (jMesh["RenderColor"][0].is_number() && jMesh["RenderColor"][1].is_number() && jMesh["RenderColor"][2].is_number() &&
			jMesh["RenderColor"][3].is_number())
		{
			Layer.vRenderColor.x = jMesh["RenderColor"][0].get<_float>();
			Layer.vRenderColor.y = jMesh["RenderColor"][1].get<_float>();
			Layer.vRenderColor.z = jMesh["RenderColor"][2].get<_float>();
			Layer.vRenderColor.w = jMesh["RenderColor"][3].get<_float>();
		}
	}

	if (jMesh.contains("EmissiveColor") && jMesh["EmissiveColor"].is_array() && jMesh["EmissiveColor"].size() == 4)
	{
		if (jMesh["EmissiveColor"][0].is_number() && jMesh["EmissiveColor"][1].is_number() && jMesh["EmissiveColor"][2].is_number() &&
			jMesh["EmissiveColor"][3].is_number())
		{
			Layer.vEmissiveColor.x = jMesh["EmissiveColor"][0].get<_float>();
			Layer.vEmissiveColor.y = jMesh["EmissiveColor"][1].get<_float>();
			Layer.vEmissiveColor.z = jMesh["EmissiveColor"][2].get<_float>();
			Layer.vEmissiveColor.w = jMesh["EmissiveColor"][3].get<_float>();
		}
	}

	if (jMesh.contains("MRAValue") && jMesh["MRAValue"].is_array() && jMesh["MRAValue"].size() == 3)
	{
		if (jMesh["MRAValue"][0].is_number() && jMesh["MRAValue"][1].is_number() && jMesh["MRAValue"][2].is_number())
		{
			Layer.vMRA.x = jMesh["MRAValue"][0].get<_float>();
			Layer.vMRA.y = jMesh["MRAValue"][1].get<_float>();
			Layer.vMRA.z = jMesh["MRAValue"][2].get<_float>();
		}
	}

	if (jMesh.contains("UseUVTransform") && jMesh["UseUVTransform"].is_boolean())
	{
		Layer.bUseUVTransform = jMesh["UseUVTransform"].get<bool>();

		if (Layer.bUseUVTransform)
		{
			JsonUtils::Try_ReadFloat2Array(jMesh, "UVScale", &Layer.vUVScale);
			JsonUtils::Try_ReadFloat2Array(jMesh, "UVScaleNormal", &Layer.vUVScaleNormal);
			JsonUtils::Try_ReadFloat2Array(jMesh, "UVScaleMaterial", &Layer.vUVScaleMaterial);

			if (jMesh.contains("UVRotate") && jMesh["UVRotate"].is_number())
				Layer.fUVRotate = jMesh["UVRotate"].get<_float>();

			JsonUtils::Try_ReadFloat2Array(jMesh, "UVOffset", &Layer.vUVOffset);
		}
	}

	if (jMesh.contains("NormalStrength") && jMesh["NormalStrength"].is_number())
		Layer.fNormalStrength = jMesh["NormalStrength"].get<_float>();

	if (jMesh.contains("MaskStrength") && jMesh["MaskStrength"].is_number())
		Layer.fMaskStrength = jMesh["MaskStrength"].get<_float>();

	if (jMesh.contains("ExtraTexType") && jMesh["ExtraTexType"].is_array() && jMesh["ExtraTexType"].size() == 4)
	{
		for (_uint i = 0; i < 4; ++i)
		{
			if (jMesh["ExtraTexType"][i].is_number_unsigned())
			{
				Layer.iExtraTexType[i] = jMesh["ExtraTexType"][i].get<unsigned int>();
			}
			else if (jMesh["ExtraTexType"][i].is_number_integer())
			{
				const int iValue = jMesh["ExtraTexType"][i].get<int>();
				if (0 <= iValue)
					Layer.iExtraTexType[i] = static_cast<unsigned int>(iValue);
			}
		}
	}

	if (jMesh.contains("ExtraBind") && jMesh["ExtraBind"].is_array() && jMesh["ExtraBind"].size() == 4)
	{
		for (_uint i = 0; i < 4; ++i)
		{
			if (!jMesh["ExtraBind"][i].is_number_integer())
				continue;

			const int iValue = jMesh["ExtraBind"][i].get<int>();
			Layer.iExtraBind[i] = (iValue >= 0) ? iValue : -1;
		}
	}

	for (_uint iTexType = 0; iTexType < MTEX_TYPE_MAX; ++iTexType)
	{
		const auto IterTexture = jMesh.find(to_string(iTexType));
		if (IterTexture == jMesh.end())
			continue;

		if (IterTexture->is_number_unsigned())
		{
			Layer.idx[iTexType] = IterTexture->get<_uint>();
		}
		else if (IterTexture->is_number_integer())
		{
			const _int iTextureIndex = IterTexture->get<_int>();
			if (0 <= iTextureIndex)
				Layer.idx[iTexType] = static_cast<_uint>(iTextureIndex);
		}
	}

	const auto IterLayerEx = jMesh.find("LayerEx");
	if (IterLayerEx != jMesh.end())
		Load_MeshLayerEx(*IterLayerEx, &Layer);

	*pOutLayer = Layer;
	return S_OK;
}

NS_END