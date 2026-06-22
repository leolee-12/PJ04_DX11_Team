#pragma once
#include "Base.h"
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

class CLevelDesign_Parser final
{
public:
	static HRESULT Parse_File(const _wstring& strJsonPath, LD_PACKAGE* pOutPackage);
	static HRESULT Parse_Root(const json& jRoot, const _wstring& wstrSourcePath, LD_PACKAGE* pOutPackage);

private:
	static void Parse_ObjectSection(const _wstring& wstrSourcePath, const _wstring& strSourceFile,
		const _wstring& strSection, const json& jSection, vector<LD_OBJECT_ENTRY>* pOutDescs);

	static void Parse_StepLinkInfo(const json& jArray, vector<LD_STEP_LINK_INFO>* pOutStepLinks);

	static LD_OBJECT_DESC Make_BaseDesc(const _wstring& wstrSourcePath, const _wstring& strSourceFile,
		const _wstring& strSection, const _wstring& strEntryKey, const json& jEntry);

	static void Fill_Common(const json& jEntry, LD_OBJECT_DESC* pDesc);
	static void Fill_SpecialFields(const json& jEntry, LD_PARSED_OBJECT* pDesc);
	static void Fill_LadderFields(const json& jEntry, LD_LADDER_DESC* pDesc);
	static void Fill_FoodFields(const json& jEntry, LD_FOOD_DESC* pDesc);
	static void Fill_PointFields(const json& jEntry, LD_POINT_DESC* pDesc);
	static void Fill_BushFields(const json& jEntry, LD_BUSH_DESC* pDesc);

private:
	static void Build_TransformDesc(LD_OBJECT_DESC* pDesc);
};

NS_END