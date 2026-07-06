#include "Timer_Manager.h"
#include "Timer.h"

CTimer_Manager::CTimer_Manager()
{
}


_float CTimer_Manager::Get_TimeDelta(const _wstring& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (!pTimer) return 0.f;

	_float fDelta = pTimer->Get_TimeDelta();
	fDelta = min(fDelta, m_fMaxTimeDelta);
	return fDelta * m_fTimeScale;
}

_float CTimer_Manager::Get_RawTimeDelta(const _wstring& strTimerTag)
{
	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return 0.f;

	_float fDelta = pTimer->Get_TimeDelta();
	fDelta = min(fDelta, m_fMaxTimeDelta);
	return fDelta;
}

void CTimer_Manager::Lerp_TimeScale(_float fFrom, _float fTo, _float fDuration)
{
	m_bScalePulse = false;

	if (fDuration <= 0.f)
	{
		m_fTimeScale = fTo;
		m_bScaleBlending = false;
		return;
	}
	m_fScaleFrom = fFrom;
	m_fScaleTo = fTo;
	m_fScaleBlendDur = fDuration;
	m_ScaleBlendStart = std::chrono::steady_clock::now();
	m_fTimeScale = fFrom;
	m_bScaleBlending = true;
}

void CTimer_Manager::Pulse_TimeScale(_float fScale, _float fHold, _float fBlendIn, _float fBlendOut)
{
	m_bScaleBlending = false;

	if (!m_bScalePulse)
		m_fPulseRestore = m_fTimeScale;

	m_bScalePulse = true;
	m_fPulseScale = fScale;
	m_fPulseIn = max(0.f, fBlendIn);
	m_fPulseHold = max(0.f, fHold);
	m_fPulseOut = max(0.f, fBlendOut);
	m_PulseStart = chrono::steady_clock::now();
}

HRESULT CTimer_Manager::Add_Timer(const _wstring& strTimerTag)
{
	if (nullptr != Find_Timer(strTimerTag))
		return E_FAIL;	

	CTimer*	pTimer = CTimer::Create();
	if (nullptr == pTimer)
		return E_FAIL;

	m_Timers.emplace(strTimerTag, pTimer);

	return S_OK;
}

void CTimer_Manager::Compute_Timer(const _wstring& strTimerTag)
{
	Tick_ScaleBlend();
	Tick_ScalePulse();

	CTimer* pTimer = Find_Timer(strTimerTag);
	if (nullptr == pTimer)
		return;

	pTimer->Update_Timer();
}

CTimer* CTimer_Manager::Find_Timer(const _wstring& strTimerTag)
{
	auto	iter = m_Timers.find(strTimerTag);

	if (iter == m_Timers.end())
		return nullptr;

	return iter->second;
}

void CTimer_Manager::Tick_ScaleBlend()
{
	if (!m_bScaleBlending)
		return;

	const _float fElapsed = std::chrono::duration<_float>(
		std::chrono::steady_clock::now() - m_ScaleBlendStart).count();

	_float t = fElapsed / m_fScaleBlendDur;
	if (t >= 1.f)
	{
		m_fTimeScale = m_fScaleTo;
		m_bScaleBlending = false;
		return;
	}
	t = t * t * (3.f - 2.f * t);
	m_fTimeScale = m_fScaleFrom + (m_fScaleTo - m_fScaleFrom) * t;
}

void CTimer_Manager::Tick_ScalePulse()
{
	if (!m_bScalePulse)
		return;

	_float e = chrono::duration<_float>(chrono::steady_clock::now() - m_PulseStart).count();

	auto Smooth = [](_float t) { return t * t * (3.f - 2.f * t); };

	if (e < m_fPulseIn)
	{
		_float t = Smooth(e / m_fPulseIn);
		m_fTimeScale = m_fPulseRestore + (m_fPulseScale - m_fPulseRestore) * t;
		return;
	}
	e -= m_fPulseIn;

	if (e < m_fPulseHold)
	{
		m_fTimeScale = m_fPulseScale;
		return;
	}
	e -= m_fPulseHold;

	if (e < m_fPulseOut)
	{
		_float t = Smooth(e / m_fPulseOut);
		m_fTimeScale = m_fPulseScale + (m_fPulseRestore - m_fPulseScale) * t;
		return;
	}

	m_fTimeScale = m_fPulseRestore;
	m_bScalePulse = false;
}

CTimer_Manager* CTimer_Manager::Create()
{
	return new CTimer_Manager();
}

void CTimer_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_Timers)	
		Safe_Release(Pair.second);
	m_Timers.clear();

}
