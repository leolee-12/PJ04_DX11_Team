#pragma once

#include "Base.h"
#include <chrono>

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

	void   Set_TimeScale(_float fTimeScale)
	{
		m_bScaleBlending = false;
		m_bScalePulse = false;
		m_fTimeScale = fTimeScale;
	}
	_float Get_TimeScale() const			{ return m_fTimeScale; }

	void   Lerp_TimeScale(_float fFrom, _float fTo, _float fDuration);
	void   Lerp_TimeScale(_float fTo, _float fDuration) { Lerp_TimeScale(m_fTimeScale, fTo, fDuration); }
	void   Pulse_TimeScale(_float fScale, _float fHold, _float fBlendIn = 0.f, _float fBlendOut = 0.f);

public:
	HRESULT	Add_Timer(const _wstring& strTimerTag);
	void Compute_Timer(const _wstring& strTimerTag);


private:
	unordered_map<_wstring, CTimer*>	m_Timers;
	_float m_fTimeScale = { 1.f };
	_float m_fMaxTimeDelta = { 0.1f };

	_bool  m_bScaleBlending = { false };
	_float m_fScaleFrom = { 1.f };
	_float m_fScaleTo = { 1.f };
	_float m_fScaleBlendDur = { 0.f };
	chrono::steady_clock::time_point m_ScaleBlendStart;

	_bool  m_bScalePulse = { false };
	_float m_fPulseScale = { 1.f };
	_float m_fPulseRestore = { 1.f };
	_float m_fPulseIn = { 0.f };
	_float m_fPulseHold = { 0.f };
	_float m_fPulseOut = { 0.f };
	chrono::steady_clock::time_point m_PulseStart;

private:
	CTimer* Find_Timer(const _wstring& strTimerTag);
	void   Tick_ScaleBlend();
	void   Tick_ScalePulse();

public:
	static CTimer_Manager* Create();
	virtual void		Free();
};

NS_END