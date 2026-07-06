#pragma once
#include "Engine_Defines.h"

#include <functional>

NS_BEGIN(Engine)

namespace JsonUtils
{
	inline vector<_string> Split_DottedPath(const _string& strPath)
	{
		vector<_string> Parts;

		size_t iStart = 0;
		while (iStart < strPath.size())
		{
			const size_t iDot = strPath.find('.', iStart);
			if (_string::npos == iDot)
			{
				Parts.push_back(strPath.substr(iStart));
				break;
			}

			Parts.push_back(strPath.substr(iStart, iDot - iStart));
			iStart = iDot + 1;
		}

		return Parts;
	}

	inline const json* Find_JsonValue(const json& jSource, const _string& strPath)
	{
		auto ExactIter = jSource.find(strPath);
		if (ExactIter != jSource.end())
			return &(*ExactIter);

		const vector<_string> Parts = Split_DottedPath(strPath);

		function<const json* (const json&, size_t)> FindRecursive;
		FindRecursive = [&](const json& Current, size_t iStart) -> const json*
			{
				if (iStart >= Parts.size())
					return &Current;

				if (!Current.is_object())
					return nullptr;

				for (size_t iEnd = Parts.size(); iEnd > iStart; --iEnd)
				{
					_string strKey;

					for (size_t i = iStart; i < iEnd; ++i)
					{
						if (i > iStart)
							strKey += ".";

						strKey += Parts[i];
					}

					auto Iter = Current.find(strKey);
					if (Iter == Current.end())
						continue;

					if (const json* pFound = FindRecursive(*Iter, iEnd))
						return pFound;
				}

				return nullptr;
			};

		return FindRecursive(jSource, 0);
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
		if (nullptr == pValue || !pValue->is_array() || pValue->size() < 3)
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
		if (nullptr == pValue || !pValue->is_array() || pValue->size() < 4)
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