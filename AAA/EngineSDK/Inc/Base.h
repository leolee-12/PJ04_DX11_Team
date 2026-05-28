#pragma once

#include "Engine_Defines.h"

/* 레퍼런스 카운트의 관리 기능(증, 감)을 모든 자식클래스에게 상속시킨다. */

NS_BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	CBase();
	CBase(const CBase&);
	virtual ~CBase() = default;

public:
	/* 레퍼런스 카운트를 증가시킨다. */ 
	_uint AddRef();

	/* 레퍼런스 카운트를 감소시킨다. or 삭제한다. */
	_uint Release();

protected:
	atomic<_uint>			m_iRefCnt = {};

public:
	virtual void Free();
};

NS_END