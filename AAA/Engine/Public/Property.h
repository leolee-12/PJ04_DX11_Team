#pragma once

#include "Engine_Defines.h"
#include <cstddef>

namespace Engine {
	enum class PROP_TYPE { INT, UINT, FLOAT, BOOL, FLOAT2, FLOAT3, FLOAT4, ANIM_INDEX, WSTRING };

	typedef struct ENGINE_DLL tagProperty
	{
		wstring		strName;
		wstring		strCategory;
		PROP_TYPE	eType = {};
		size_t		uOffset = {};
	}FPROPERTY;
}

typedef _uint  ANIM_INDEX;

#define _wstring_TYPE	Engine::PROP_TYPE::WSTRING
#define wstring_TYPE	Engine::PROP_TYPE::WSTRING
#define ANIM_INDEX_TYPE Engine::PROP_TYPE::ANIM_INDEX
#define int_TYPE		Engine::PROP_TYPE::INT
#define _int_TYPE		Engine::PROP_TYPE::INT
#define _uint_TYPE		Engine::PROP_TYPE::UINT
#define float_TYPE		Engine::PROP_TYPE::FLOAT
#define _float_TYPE		Engine::PROP_TYPE::FLOAT
#define bool_TYPE		Engine::PROP_TYPE::BOOL
#define _bool_TYPE		Engine::PROP_TYPE::BOOL
#define _float2_TYPE	Engine::PROP_TYPE::FLOAT2
#define _float3_TYPE	Engine::PROP_TYPE::FLOAT3
#define _float4_TYPE	Engine::PROP_TYPE::FLOAT4

template<typename T, typename M>
constexpr size_t offset_of(M T::* member) {
	return reinterpret_cast<size_t>(&(((T*)0)->*member));
}

#define GENERATED_BODY(ClassName) \
public: \
	using ThisClass = ClassName; \
	static vector<FPROPERTY>& Get_StaticProperties() { \
		static vector<Engine::FPROPERTY> props; \
		return props; \
	} \
	virtual vector<Engine::FPROPERTY>& Get_Properties() const override { \
		static vector<Engine::FPROPERTY> merged; \
		static bool bInitialized = false; \
		if (!bInitialized) { \
		  bInitialized = true; \
		  merged = __super::Get_Properties(); \
		  for (auto& p : Get_StaticProperties()) \
		    merged.push_back(p); \
		} \
		return merged; \
	} \
	virtual const void* Get_PropertyPtr(size_t uOffset) const override { \
		return (const char*)this + uOffset; \
	} \
	virtual void* Get_PropertyPtr(size_t uOffset) override { \
		return (char*)this + uOffset; \
	} \
private:

#define GENERATED_BODY_ABSTRACT(ClassName) \
public: \
	using ThisClass = ClassName; \
	static vector<FPROPERTY>& Get_StaticProperties() { \
		static vector<Engine::FPROPERTY> props; \
		return props; \
	} \
	virtual vector<Engine::FPROPERTY>& Get_Properties() const override { \
		static vector<Engine::FPROPERTY> merged; \
		static bool bInitialized = false; \
		if (!bInitialized) { \
		  bInitialized = true; \
		  merged = __super::Get_Properties(); \
		  for (auto& p : Get_StaticProperties()) \
		    merged.push_back(p); \
		} \
		return merged; \
	} \
	virtual const void* Get_PropertyPtr(size_t uOffset) const override { \
		return (const char*)this + uOffset; \
	} \
	virtual void* Get_PropertyPtr(size_t uOffset) override { \
		return (char*)this + uOffset; \
	} \
protected:

#define PROPERTY(Type, Var, Name, Category)                  \
    Type Var;                                                \
    struct FRegister_##Var {                                 \
        FRegister_##Var() {                                  \
            RegisterProperty();                              \
        }                                                    \
        static void RegisterProperty() {                     \
            Get_StaticProperties().push_back({               \
                Name,                                        \
                Category,                                    \
                Type##_TYPE,								 \
                offset_of(&ThisClass::Var)                   \
            });                                              \
        }                                                    \
    };                                                       \
    inline static FRegister_##Var _reg_##Var{};




