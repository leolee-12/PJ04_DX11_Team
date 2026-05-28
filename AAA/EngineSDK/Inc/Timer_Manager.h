#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CTimer;

class CTimer_Manager final : public CBase
{
private:
	CTimer_Manager();
	virtual ~CTimer_Manager() = default;

public:
	_float Get_TimeDelta(const _wstring& strTimerTag);
	_float Get_RawTimeDelta(const _wstring& strTimerTag);

	void   Set_TimeScale(_float fTimeScale) { m_fTimeScale = fTimeScale; }
	_float Get_TimeScale() const			{ return m_fTimeScale; }

public:
	HRESULT	Add_Timer(const _wstring& strTimerTag);
	void Compute_Timer(const _wstring& strTimerTag);


private:
	unordered_map<_wstring, CTimer*>	m_Timers;
	_float m_fTimeScale = { 1.f };

private:
	CTimer* Find_Timer(const _wstring& strTimerTag);

public:
	static CTimer_Manager* Create();
	virtual void		Free();
};

NS_END