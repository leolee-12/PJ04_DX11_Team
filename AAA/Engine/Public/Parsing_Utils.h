#pragma once
#include "Engine_Defines.h"

#include <string_view>

NS_BEGIN(Engine)

namespace JsonUtils
{
	inline const json* Find_JsonValueByPath(const json& jSource, std::string_view strPath)
	{
		if (strPath.empty())
			return &jSource;

		if (!jSource.is_object())
			return nullptr;

		for (size_t iKeyLength = strPath.size(); ; )
		{
			auto Iter = jSource.find(strPath.substr(0, iKeyLength));
			if (Iter != jSource.end())
			{
				if (iKeyLength == strPath.size())
					return &(*Iter);

				if (const json* pFound = Find_JsonValueByPath(*Iter, strPath.substr(iKeyLength + 1)))
					return pFound;
			}

			if (0 == iKeyLength)
				break;

			const size_t iDot = strPath.rfind('.', iKeyLength - 1);
			if (std::string_view::npos == iDot)
				break;

			iKeyLength = iDot;
		}

		return nullptr;
	}

	inline const json* Find_JsonValue(const json& jSource, const _string& strPath)
	{
		auto ExactIter = jSource.find(strPath);
		if (ExactIter != jSource.end())
			return &(*ExactIter);

		return Find_JsonValueByPath(jSource, std::string_view(strPath));
	}

	inline _bool Has_NumberArrayElements(const json& jValue, size_t iRequiredCount)
	{
		if (!jValue.is_array() || jValue.size() < iRequiredCount)
			return false;

		for (size_t i = 0; i < iRequiredCount; ++i)
		{
			if (!jValue[i].is_number())
				return false;
		}

		return true;
	}

	inline _bool Try_ReadString(const json& jSource, const _string& strPath, _wstring* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_string())
			return false;

		*pOut = StrToWstr(pValue->get<string>());
		return true;
	}

	inline _bool Try_ReadUInt(const json& jSource, const _string& strPath, _uint* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue)
			return false;

		if (pValue->is_number_unsigned())
		{
			*pOut = pValue->get<_uint>();
			return true;
		}

		if (pValue->is_number_integer())
		{
			const _int iValue = pValue->get<_int>();
			if (0 > iValue)
				return false;

			*pOut = static_cast<_uint>(iValue);
			return true;
		}

		return false;
	}

	inline _bool Try_ReadInt(const json& jSource, const _string& strPath, _int* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_number_integer())
			return false;

		*pOut = pValue->get<_int>();
		return true;
	}

	inline _bool Try_ReadFloat(const json& jSource, const _string& strPath, _float* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_number())
			return false;

		*pOut = pValue->get<_float>();
		return true;
	}

	inline _bool Try_ReadBoolFromNumeric(const json& jSource, const _string& strPath, _bool* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue)
			return false;

		if (pValue->is_boolean())
		{
			*pOut = pValue->get<_bool>();
			return true;
		}

		if (pValue->is_number_integer())
		{
			*pOut = 0 != pValue->get<_int>();
			return true;
		}

		if (pValue->is_number_unsigned())
		{
			*pOut = 0u != pValue->get<_uint>();
			return true;
		}

		return false;
	}

	inline _bool Try_ReadFloat3Array(const json& jSource, const _string& strPath, _float3* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !Has_NumberArrayElements(*pValue, 3))
			return false;

		pOut->x = (*pValue)[0].get<_float>();
		pOut->y = (*pValue)[1].get<_float>();
		pOut->z = (*pValue)[2].get<_float>();
		return true;
	}

	inline _bool Try_ReadFloat4Array(const json& jSource, const _string& strPath, _float4* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !Has_NumberArrayElements(*pValue, 4))
			return false;

		pOut->x = (*pValue)[0].get<_float>();
		pOut->y = (*pValue)[1].get<_float>();
		pOut->z = (*pValue)[2].get<_float>();
		pOut->w = (*pValue)[3].get<_float>();
		return true;
	}

	inline _bool Try_ReadFloat3List(const json& jSource, const _string& strPath, vector<_float3>* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_array())
			return false;

		pOut->clear();

		if (pValue->empty())
			return true;

		if ((*pValue)[0].is_array())
		{
			pOut->reserve(pValue->size());

			for (const json& jItem : *pValue)
			{
				if (!jItem.is_array() || jItem.size() < 3)
					return false;

				_float3 Value{};
				Value.x = jItem[0].get<_float>();
				Value.y = jItem[1].get<_float>();
				Value.z = jItem[2].get<_float>();
				pOut->push_back(Value);
			}

			return true;
		}

		if (0 != (pValue->size() % 3u))
			return false;

		pOut->reserve(pValue->size() / 3u);

		for (size_t i = 0; i < pValue->size(); i += 3u)
		{
			if (!(*pValue)[i + 0].is_number()
				|| !(*pValue)[i + 1].is_number()
				|| !(*pValue)[i + 2].is_number())
			{
				return false;
			}

			_float3 Value{};
			Value.x = (*pValue)[i + 0].get<_float>();
			Value.y = (*pValue)[i + 1].get<_float>();
			Value.z = (*pValue)[i + 2].get<_float>();
			pOut->push_back(Value);
		}

		return true;
	}

	inline _bool Try_ReadStringArray(const json& jSource, const _string& strPath, vector<_wstring>* pOut)
	{
		if (nullptr == pOut)
			return false;

		const json* pValue = Find_JsonValue(jSource, strPath);
		if (nullptr == pValue || !pValue->is_array())
			return false;

		pOut->clear();
		pOut->reserve(pValue->size());

		for (const json& jItem : *pValue)
		{
			if (!jItem.is_string())
				return false;

			pOut->push_back(StrToWstr(jItem.get<string>()));
		}

		return true;
	}

	inline _bool Equals_NoCase(const _tchar* strLeft, const _tchar* pRight)
	{
		return 0 == _wcsicmp(strLeft, pRight);
	}
}

NS_END