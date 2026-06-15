#pragma once
#include "Base.h"
#include <variant>

NS_BEGIN(Engine)

using BB_VALUE = variant<_bool, _int, _uint, _float, _float2, _float3, _float4, void*>;

class ENGINE_DLL CBlackboard final : public CBase
{
private:
	CBlackboard() = default;
	virtual ~CBlackboard() = default;

public:
	template<class T> void Set(const string& strKey, const T& tValue) 
	{ 
		m_Values[strKey] = tValue; 
	}
	template<class T> T Get(const string& strKey, const T& tDefault = {}) const
	{
		auto it = m_Values.find(strKey);
		if (it == m_Values.end())
			return tDefault;
		if (const T* p = get_if<T>(&it->second))  // 타입 안 맞으면 nullptr
			return *p;
		return tDefault;
	}
	_bool Has(const string& strKey) const { return m_Values.count(strKey) != 0; }
	void  Erase(const string& strKey) { m_Values.erase(strKey); }

private:
	unordered_map<string, BB_VALUE> m_Values;

public:
	static CBlackboard* Create();
	virtual void Free() override;
};

NS_END

