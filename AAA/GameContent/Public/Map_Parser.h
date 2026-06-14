#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)

class CMap_Parser final
{
public:
	static HRESULT Parse_Manifest(const _wstring& strManifestPath, MAP_MANIFEST_DESC* pOutManifest);
	static HRESULT Parse_EnvJson(const _wstring& strJsonPath, vector<ENV_OBJECT_DESC>* pOutDescs);

private:
	static HRESULT Parse_ManifestRoot(
		const json& jRoot,
		const filesystem::path& ManifestPath,
		MAP_MANIFEST_DESC* pOutManifest);

	static HRESULT Parse_Sections(
		const json& jManifest,
		MAP_MANIFEST_DESC* pOutManifest);

	static HRESULT Parse_SectionEntry(
		const json& jSection,
		MAP_MANIFEST_DESC* pOutManifest);

	static HRESULT Parse_EnvRoot(const json& jRoot, vector<ENV_OBJECT_DESC>* pOutDescs);

	static void Parse_SectionObject(
		const _wstring& wstrSourceFile,
		const _wstring& wstrSection,
		const json& jSection,
		vector<ENV_OBJECT_DESC>* pOutDescs);

	static void Parse_DecorEntry(
		const _wstring& wstrSourceFile,
		const _wstring& wstrSection,
		const _wstring& wstrEntryKey,
		const json& jEntry,
		vector<ENV_OBJECT_DESC>* pOutDescs);

	static void Parse_ToyObjEntry(
		const _wstring& wstrSourceFile,
		const _wstring& wstrSection,
		const _wstring& wstrEntryKey,
		const json& jEntry,
		vector<ENV_OBJECT_DESC>* pOutDescs);

	static void Parse_EffectEntry(
		const _wstring& wstrSourceFile,
		const _wstring& wstrSection,
		const _wstring& wstrEntryKey,
		const json& jEntry,
		vector<ENV_OBJECT_DESC>* pOutDescs);

private:
	static ENV_OBJECT_DESC Make_BaseDesc(
		const _wstring& wstrSourceFile,
		const _wstring& wstrSection,
		const _wstring& wstrEntryKey);

	static ENV_SOURCE_TYPE Classify_SourceType(const _wstring& wstrSourceFile);
	static ENV_EFFECT_TYPE Classify_EffectType(const _wstring& wstrObjectName, const _wstring&
		wstrComponentName);

	static const json* Find_JsonValue(const json& jSource, const _string& strPath);

	static _bool Try_ReadString(const json& jSource, const _string& strPath, wstring* pOut);
	static _bool Try_ReadUInt(const json& jSource, const _string& strPath, _uint* pOut);
	static _bool Try_ReadFloat(const json& jSource, const _string& strPath, _float* pOut);
	static _bool Try_ReadFloat3Array(const json& jSource, const _string& strPath, _float3* pOut);
	static _bool Try_ReadFloat4Array(const json& jSource, const _string& strPath, _float4* pOut);
	static _bool Try_ReadBoolFromNumeric(const json& jSource, const _string& strPath, _bool* pOut);
	static _bool Try_ReadStringArray(const json& jSource, const _string& strPath, vector<_wstring>* pOut);
	static _bool Try_ParseSectionType(const json& jValue, MAP_SECTION_TYPE* pOut);
	static _bool Try_ParseRenderID(const json& jValue, RENDERID* pOut);
	static _bool Try_BuildWorldMatrixFromArray(const json& jSource, const _string& strPath, ENV_OBJECT_DESC* pOutDesc);

	static _wstring Resolve_PathFromManifest(const filesystem::path& ManifestPath, const _wstring& strRawPath);

	static void Fill_CommonFlags(const json& jEntry, ENV_OBJECT_DESC* pDesc);
};

NS_END