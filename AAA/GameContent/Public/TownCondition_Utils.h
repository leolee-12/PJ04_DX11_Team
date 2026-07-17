#pragma once
#include "Parsing_Utils.h"

NS_BEGIN(Client)

namespace TownConditionUtils
{
	inline _bool Try_ReadRule(const json& jEntry, const _string& strRuleName, _wstring* pOutRule)
	{
		if (nullptr == pOutRule)
			return false;

		if (JsonUtils::Try_ReadString(jEntry, strRuleName, pOutRule))
			return true;
		if (JsonUtils::Try_ReadString(jEntry, "Basic.BasicInfo." + strRuleName, pOutRule))
			return true;

		return JsonUtils::Try_ReadString(jEntry, "Basic.TownCreateConditionExporter." + strRuleName, pOutRule);
	}

	inline _bool Is_Enabled_AllUnlocked(const _wstring& strRule)
	{
		return strRule.empty()
			|| JsonUtils::Equals_NoCase(strRule.c_str(), L"None")
			|| JsonUtils::Equals_NoCase(strRule.c_str(), L"Unlocked");
	}

	inline _bool Passes_AllUnlocked(const json& jEntry)
	{
		_wstring strAreaRule = L"None";
		_wstring strBuildingRule = L"None";

		Try_ReadRule(jEntry, "TownAreaCreateRule", &strAreaRule);
		Try_ReadRule(jEntry, "TownBuildingCreateRule", &strBuildingRule);

		return Is_Enabled_AllUnlocked(strAreaRule)
			&& Is_Enabled_AllUnlocked(strBuildingRule);
	}

	inline _bool Passes_TalkWaddlePatternA(const json& jEntry)
	{
		_wstring strObjectName;
		if (!JsonUtils::Try_ReadString(jEntry, "Basic.ObjectName", &strObjectName))
			JsonUtils::Try_ReadString(jEntry, "Basic.BasicInfo.ObjectName", &strObjectName);

		if (!JsonUtils::Equals_NoCase(strObjectName.c_str(), L"TalkWaddleDee"))
			return true;

		_wstring strGenerateType;
		if (!JsonUtils::Try_ReadString(jEntry, "Basic.TownGenerateInfo.GenerateType", &strGenerateType))
			return false;

		return JsonUtils::Equals_NoCase(strGenerateType.c_str(), L"RandomPatternA");
	}
}

NS_END