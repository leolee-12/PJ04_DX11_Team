#include "Map_Override.h"

#include <algorithm>

NS_BEGIN(Client)

_wstring CMap_Override::Build_EnvObjectStableKey(const ENV_OBJECT_DESC& Desc)
{
    return Desc.strSourceFile + L"|"
        + Desc.strSection + L"|"
        + Desc.strEntryKey + L"|"
        + to_wstring(Desc.iUid);
}

json CMap_Override::Serialize(const MAP_OVERRIDE_DESC& Desc)
{
    json jOverride = json::object();
    jOverride["Version"] = Desc.Version;

    vector<_wstring> DeletedKeys(
        Desc.DeletedEnvObjectKeys.begin(),
        Desc.DeletedEnvObjectKeys.end());

    sort(DeletedKeys.begin(), DeletedKeys.end());

    json jDeletedEnvObjects = json::array();
    for (const auto& strKey : DeletedKeys)
        jDeletedEnvObjects.push_back(WstrToStr(strKey));

    jOverride["DeletedEnvObjects"] = jDeletedEnvObjects;

    jOverride["AddedMapObjects"] = json::array();
    for (const auto& Added : Desc.AddedMapObjects)
    {
        json jAdded = json::object();
        jAdded["Prototype_Tag"] = WstrToStr(Added.strPrototypeTag);
        jAdded["Layer_Tag"] = WstrToStr(Added.strLayerTag);
        jAdded["Object_Tag"] = WstrToStr(Added.strObjectTag);
        jAdded["Object"] = Added.jObject;
        jOverride["AddedMapObjects"].push_back(jAdded);
    }

    return jOverride;
}

HRESULT CMap_Override::Deserialize(const json& jOverride, MAP_OVERRIDE_DESC* pOutDesc)
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

    const auto IterAdded = jOverride.find("AddedMapObjects");
    if (IterAdded != jOverride.end())
    {
        if (!IterAdded->is_array())
            return E_FAIL;

        for (const auto& jAdded : *IterAdded)
        {
            if (!jAdded.is_object())
                continue;

            MAP_ADDED_OBJECT_DESC AddedDesc{};
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

HRESULT CMap_Override::Apply(MAP_PACKAGE* pInOutPackage, const MAP_OVERRIDE_DESC& OverrideDesc)
{
    if (nullptr == pInOutPackage)
        return E_FAIL;

    if (!OverrideDesc.DeletedEnvObjectKeys.empty())
    {
        auto& EnvObjectDescs = pInOutPackage->EnvObjectDescs;
        EnvObjectDescs.erase(
            remove_if(
                EnvObjectDescs.begin(),
                EnvObjectDescs.end(),
                [&](const ENV_OBJECT_DESC& Desc)
                {
                    const _wstring strKey = Build_EnvObjectStableKey(Desc);
                    return OverrideDesc.DeletedEnvObjectKeys.find(strKey)
                        != OverrideDesc.DeletedEnvObjectKeys.end();
                }),
            EnvObjectDescs.end());
    }

    if (!OverrideDesc.AddedMapObjects.empty())
    {
        pInOutPackage->AddedObjectDescs.insert(
            pInOutPackage->AddedObjectDescs.end(),
            OverrideDesc.AddedMapObjects.begin(),
            OverrideDesc.AddedMapObjects.end());
    }

    return S_OK;
}

NS_END