#include "Effect_Allocator.h"
#include <malloc.h>

IMPLEMENT_SINGLETON(CEffect_Allocator)

static thread_local int g_iPrototypeDepth = 0;

CEffect_Allocator::CEffect_Allocator()
{
	m_pBase = static_cast<_byte*>(_aligned_malloc(ARENA, 16));
}

void* CEffect_Allocator::Alloc(size_t n) {
	size_t a = (n + 15) & ~size_t(15);
	if (m_pBase && m_uBump + a <= ARENA) 
	{ 
		void* p = m_pBase + m_uBump;
		m_uBump += a; return p; 
	}
#pragma push_macro("new")
#undef new
	return ::operator new(n);     // 폴백 전역 (← _aligned_malloc 대신)
#pragma pop_macro("new")
}

void CEffect_Allocator::Dealloc(void* p) {
	if (m_pBase && p >= m_pBase && p < m_pBase + ARENA) return;
	::operator delete(p);
}

void CEffect_Allocator::Reset()
{
	m_uBump = 0;
}

void CEffect_Allocator::Push_PrototypePass() 
{ 
	++g_iPrototypeDepth; 
}

void CEffect_Allocator::Pop_PrototypePass() 
{ 
	--g_iPrototypeDepth; 
}

bool CEffect_Allocator::IsPrototypePass() 
{ 
	return g_iPrototypeDepth > 0; 
}

void CEffect_Allocator::Free()
{
	_aligned_free(m_pBase);
	m_pBase = nullptr;
	__super::Free();
}