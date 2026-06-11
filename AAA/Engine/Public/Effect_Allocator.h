#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEffect_Allocator final : public CBase
{
	DECLARE_SINGLETON(CEffect_Allocator)

private:
	CEffect_Allocator();
	virtual ~CEffect_Allocator() = default;

public:
	void* Alloc(size_t n);
	void  Dealloc(void* p);
	void  Reset();

public:
	static void Push_PrototypePass();
	static void Pop_PrototypePass();
	static _bool IsPrototypePass();

	struct PrototypeScope {
		PrototypeScope() { Push_PrototypePass(); }
		~PrototypeScope() { Pop_PrototypePass(); }
	};

private:
	static constexpr size_t ARENA = { 8 * 1024 * 1024 };
	_byte* m_pBase = { nullptr };
	size_t m_uBump = { 0 };

public:
	virtual void Free() override;
};

NS_END

