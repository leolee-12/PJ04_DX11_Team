#include "Property.h"

json IReflectable::Serialize() const
{
    json j;

    for (auto& prop : Get_Properties())
    {
        const void* pData = Get_PropertyPtr(prop.uOffset);
        if (!pData) continue;

        string strKey = WstrToStr(prop.strName);

        switch (prop.eType)
        {
            case PROP_TYPE::INT:    j[strKey] = *(int*)pData;   break;
            case PROP_TYPE::UINT:   j[strKey] = *(_uint*)pData;   break;
            case PROP_TYPE::FLOAT:  j[strKey] = *(float*)pData; break;
            case PROP_TYPE::BOOL:   j[strKey] = *(bool*)pData;  break;
            case PROP_TYPE::FLOAT2: { _float2* v = (_float2*)pData; j[strKey] = { v->x, v->y }; } break;
            case PROP_TYPE::FLOAT3: { _float3* v = (_float3*)pData; j[strKey] = { v->x, v->y, v->z }; } break;
            case PROP_TYPE::FLOAT4: { _float4* v = (_float4*)pData; j[strKey] = { v->x, v->y, v->z, v->w }; } break;
            case PROP_TYPE::WSTRING: j[strKey] = WstrToStr(*(wstring*)pData); break;
        }
    }

    return j;
}

void IReflectable::Deserialize_Internal(const json& j)
{
    for (auto& prop : Get_Properties())
    {
        string strKey = WstrToStr(prop.strName);
        if (!j.contains(strKey)) continue;

        void* pData = Get_PropertyPtr(prop.uOffset);
        if (!pData) continue;

        switch (prop.eType)
        {
            case PROP_TYPE::INT:   *(int*)pData = j[strKey].get<int>();   break;
            case PROP_TYPE::UINT:  *(_uint*)pData = j[strKey].get<int>();   break;
            case PROP_TYPE::FLOAT: *(float*)pData = j[strKey].get<float>(); break;
            case PROP_TYPE::BOOL:  *(bool*)pData = j[strKey].get<bool>();  break;
            case PROP_TYPE::FLOAT2:
            {
                _float2* v = (_float2*)pData;
                v->x = j[strKey][0].get<float>();
                v->y = j[strKey][1].get<float>();
            } break;
            case PROP_TYPE::FLOAT3:
            {
                _float3* v = (_float3*)pData;
                v->x = j[strKey][0].get<float>();
                v->y = j[strKey][1].get<float>();
                v->z = j[strKey][2].get<float>();
            } break;
            case PROP_TYPE::FLOAT4:
            {
                _float4* v = (_float4*)pData;
                v->x = j[strKey][0].get<float>();
                v->y = j[strKey][1].get<float>();
                v->z = j[strKey][2].get<float>();
                v->w = j[strKey][3].get<float>();
            } break;
            case PROP_TYPE::WSTRING:
                *(wstring*)pData = StrToWstr(j[strKey].get<string>());
                break;
        }
    }
}
