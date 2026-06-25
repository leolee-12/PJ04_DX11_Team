#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)

json Save_MeshLayerBindEx(const MESH_LAYER_TEX_BIND_EX& Bind)
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

bool Has_MeshLayerBindExValue(const MESH_LAYER_TEX_BIND_EX& Bind)
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

json Save_MeshLayerEx(const MESH_LAYER_IDX& Layer)
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

bool Has_MeshLayerExValue(const MESH_LAYER_IDX& Layer)
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

void Load_MeshLayerBindEx(const json& j, MESH_LAYER_TEX_BIND_EX* pOut)
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

    if (j.contains("Scale") && j["Scale"].is_array() && j["Scale"].size() == 2)
    {
        if (j["Scale"][0].is_number() && j["Scale"][1].is_number())
        {
            Bind.vUVScale.x = j["Scale"][0].get<_float>();
            Bind.vUVScale.y = j["Scale"][1].get<_float>();
        }
    }

    if (j.contains("Offset") && j["Offset"].is_array() && j["Offset"].size() == 2)
    {
        if (j["Offset"][0].is_number() && j["Offset"][1].is_number())
        {
            Bind.vUVOffset.x = j["Offset"][0].get<_float>();
            Bind.vUVOffset.y = j["Offset"][1].get<_float>();
        }
    }

    if (j.contains("Rotate") && j["Rotate"].is_number())
        Bind.fUVRotate = j["Rotate"].get<_float>();

    *pOut = Bind;
}

void Load_MeshLayerEx(const json& jLayerEx, MESH_LAYER_IDX* pOutLayer)
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
NS_END