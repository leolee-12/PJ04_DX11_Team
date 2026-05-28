#include "../Public/Base.h"

CBase::CBase()
	: m_iRefCnt(1)
{
}

CBase::CBase(const CBase&)
	: m_iRefCnt(1)
{
}

_uint CBase::AddRef()
{
	return ++m_iRefCnt;
}

_uint CBase::Release()
{
	_uint iPrev = m_iRefCnt.load();

	if (0 == iPrev)
	{
		MSG_BOX("Reference Count is Already 0");
		return 0;
	}

	_uint iRefCnt = --m_iRefCnt;

	if (0 == iRefCnt)
	{
		Free();
		delete this;
		return 0;
	}
	else
	{
		return iRefCnt;
	}
}

void CBase::Free()
{
}
