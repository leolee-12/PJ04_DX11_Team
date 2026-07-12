#include "Sound_Manager.h"
#include "fmod/fmod.hpp"
#include <io.h>

CSound_Manager::CSound_Manager()
	: m_pSystem(nullptr)
{
}

HRESULT CSound_Manager::Initialize()
{
	if (FMOD::System_Create(&m_pSystem) != FMOD_OK)
		return E_FAIL;

	if (m_pSystem->init(SOUND_MAX_CHANNEL, FMOD_INIT_NORMAL, nullptr) != FMOD_OK)
		return E_FAIL;

	if (FAILED(Ready_Buses()))
		return E_FAIL;

	return LoadSoundFile();
}

HRESULT CSound_Manager::Ready_Buses()
{
	static const char* s_szBusName[] = { "BGM", "SFX", "UI", "VOICE", "Ambient" };

	FMOD::ChannelGroup* pMaster = nullptr;
	m_pSystem->getMasterChannelGroup(&pMaster);

	for (_uint i = 0; i < ETOUI(ESoundBus::END); ++i)
	{
		if (m_pSystem->createChannelGroup(s_szBusName[i], &m_pBuses[i]) != FMOD_OK)
			return E_FAIL;
		if (pMaster)
			pMaster->addGroup(m_pBuses[i]);
	}
	return S_OK;
}

FMOD::Sound* CSound_Manager::Find_Sound(const TCHAR* pSoundKey) const
{
	auto it = m_mapSound.find(pSoundKey);
	if (it == m_mapSound.end())
	{
		OutputDebugString(L"[Sound] not found: ");
		OutputDebugString(pSoundKey);
		OutputDebugString(L"\n");
		return nullptr;
	}
	return it->second;
}

void CSound_Manager::Update()
{
	// FMOD 시스템 업데이트 (매 프레임 호출 필요)
	if (m_pSystem)
	{
		m_pSystem->update();
	}
}

FMOD::Channel* CSound_Manager::PlayInternal(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus, bool bLoop)
{
	FMOD::Sound* pSound = Find_Sound(pSoundKey);
	if (!pSound)
		return nullptr;

	FMOD::Channel* pChannel = nullptr;
	// paused=true 로 시작 -> 세팅 후 재생(팝 방지). 채널 할당은 FMOD가 처리.
	if (m_pSystem->playSound(pSound, m_pBuses[ETOUI(eBus)], true, &pChannel) != FMOD_OK || !pChannel)
		return nullptr;

	pChannel->setVolume(fVolume);
	pChannel->setMode(bLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	pChannel->setPaused(false);
	return pChannel;
}

CSound_Handle CSound_Manager::PlaySFX(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus)
{
	return CSound_Handle(PlayInternal(pSoundKey, fVolume, eBus, false));
}

CSound_Handle CSound_Manager::PlaySFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume, ESoundBus eBus)
{
	_vector vDir = vSoundPos - XMLoadFloat3(&m_vListenerPos);
	_float fDistance = XMVectorGetX(XMVector3Length(vDir));

	_float fAtten = 1.f - (fDistance / m_fMaxDistance);
	fAtten = max(0.f, fAtten);
	if (fAtten < 0.001f)
		return CSound_Handle();

	return CSound_Handle(PlayInternal(pSoundKey, fVolume * fAtten, eBus, false));
}

CSound_Handle CSound_Manager::PlaySFXLoop(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus)
{
	return CSound_Handle(PlayInternal(pSoundKey, fVolume, eBus, true));
}

CSound_Handle CSound_Manager::PlaySFX3DLoop(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume, ESoundBus eBus)
{
	_vector vDir = vSoundPos - XMLoadFloat3(&m_vListenerPos);
	_float fDist = XMVectorGetX(XMVector3Length(vDir));
	_float fAtten = max(0.f, 1.f - fDist / m_fMaxDistance);
	return CSound_Handle(PlayInternal(pSoundKey, fVolume * fAtten, eBus, true));
}

CSound_Handle CSound_Manager::PlaySFXSectionLoop(const TCHAR* pSoundKey, float _fLoopStart, float fLoopEnd, float fVolume, ESoundBus eBus)
{
	FMOD::Sound* pSound = Find_Sound(pSoundKey);
	if (!pSound)
		return CSound_Handle();

	_uint iLenMs = 0;
	pSound->getLength(&iLenMs, FMOD_TIMEUNIT_MS);

	FMOD::Channel* pCh =
		PlayInternal(pSoundKey, fVolume, eBus, true);
	if (!pCh || 0 == iLenMs)
		return CSound_Handle(pCh);

	_fLoopStart = max(0.f, min(_fLoopStart, 1.f));
	fLoopEnd = max(0.f, min(fLoopEnd, 1.f));
	if (fLoopEnd <= _fLoopStart)
		return CSound_Handle(pCh);   // 잘못된 구간 -> 전체 반복(기본)

	_uint iStartMs = (_uint)(iLenMs * _fLoopStart);
	_uint iEndMs = (_uint)(iLenMs * fLoopEnd);
	if (iEndMs >= iLenMs)
		iEndMs = iLenMs - 1;

	pCh->setLoopPoints(
		iStartMs, FMOD_TIMEUNIT_MS,
		iEndMs, FMOD_TIMEUNIT_MS);
	return CSound_Handle(pCh);
}

void CSound_Manager::PlayBGM(const TCHAR* pSoundKey, float fVolume, bool bLoop)
{
	StopBGM();   // 기존 BGM 교체(겹침 방지)
	m_pBGMChannel = PlayInternal(pSoundKey, fVolume, ESoundBus::BGM, bLoop);
}

void CSound_Manager::StopBGM()
{
	if (m_pBGMChannel)
	{
		bool bPlaying = false;
		m_pBGMChannel->isPlaying(&bPlaying);   
		if (bPlaying)
			m_pBGMChannel->stop();
		m_pBGMChannel = nullptr;
	}
}

void CSound_Manager::PlayBGM_Section(const TCHAR* pSoundKey, float fStart01, float fEnd01, float fVolume)
{
	FMOD::Sound* pSound = Find_Sound(pSoundKey);
	if (!pSound)
		return;

	_uint iLenMs = 0;
	pSound->getLength(&iLenMs, FMOD_TIMEUNIT_MS);

	StopBGM();
	m_pBGMChannel = PlayInternal(pSoundKey, fVolume, ESoundBus::BGM, true);
	if (!m_pBGMChannel || 0 == iLenMs)
		return;

	fStart01 = max(0.f, min(fStart01, 1.f));
	fEnd01 = max(0.f, min(fEnd01, 1.f));
	if (fEnd01 <= fStart01)
		return;

	_uint iStartMs = (_uint)(iLenMs * fStart01);
	_uint iEndMs = (_uint)(iLenMs * fEnd01);
	if (iEndMs >= iLenMs)
		iEndMs = iLenMs - 1;

	m_pBGMChannel->setLoopPoints(iStartMs, FMOD_TIMEUNIT_MS, iEndMs, FMOD_TIMEUNIT_MS);
}

void CSound_Manager::Fade_BGM_Out(_float fSec)
{
	if (!m_pBGMChannel)
		return;

	Fade_BGM_Channel(m_pBGMChannel, 1.f, 0.f, fSec, true);
	m_pBGMChannel = nullptr; 
}

void CSound_Manager::Play_BGM_Fade(const TCHAR* pSoundKey, float fSec, float fVolume, CSound_Handle* pOut)
{
	Fade_BGM_Out(fSec);

	m_pBGMChannel = PlayInternal(pSoundKey, fVolume, ESoundBus::BGM, true);

	if (pOut)
		*pOut = CSound_Handle(m_pBGMChannel);

	if (!m_pBGMChannel)
		return;

	Fade_BGM_Channel(m_pBGMChannel, 0.f, 1.f, fSec, false);
}

void CSound_Manager::Resume_BGM_Fade(CSound_Handle& hBgm, float fInSec)
{
	FMOD::Channel* pChannel = hBgm.m_pChannel;
	if (!pChannel)
		return;

	bool bPlaying = false;
	if (pChannel->isPlaying(&bPlaying) != FMOD_OK || !bPlaying)
	{
		hBgm = CSound_Handle();
		return;
	}

	if (m_pBGMChannel && m_pBGMChannel != pChannel)
		Fade_BGM_Out(fInSec);

	m_pBGMChannel = pChannel;
	pChannel->setPaused(false);
	Fade_BGM_Channel(pChannel, 0.f, 1.f, fInSec, false);
}

void CSound_Manager::SetBusVolume(ESoundBus eBus, float fVolume)
{
	if (m_pBuses[ETOUI(eBus)])
		m_pBuses[ETOUI(eBus)]->setVolume(fVolume);
}

void CSound_Manager::StopBus(ESoundBus eBus)
{
	if (m_pBuses[ETOUI(eBus)])
		m_pBuses[ETOUI(eBus)]->stop();
	if (eBus == ESoundBus::BGM)
		m_pBGMChannel = nullptr;
}

void CSound_Manager::StopAll()
{
	for (_uint i = 0; i < ETOUI(ESoundBus::END); ++i)
		if (m_pBuses[i])
			m_pBuses[i]->stop();
	m_pBGMChannel = nullptr;
}

HRESULT CSound_Manager::LoadSoundFile()
{
	return LoadSoundFileRecursive("../../Resources/Sounds/");
}

HRESULT CSound_Manager::LoadSoundFileRecursive(const char* pPath)
{
	_finddata_t fd;
	char szSearchPath[256] = "";
	sprintf_s(szSearchPath, "%s*.*", pPath);

	intptr_t handle = _findfirst(szSearchPath, &fd);

	if (handle == -1)
	{
		// 경로를 찾지 못함!
		OutputDebugStringA("사운드 폴더를 찾을 수 없습니다: ");
		OutputDebugStringA(pPath);
		OutputDebugStringA("\n");
		return E_FAIL;
	}

	int iResult = 0;

	while (iResult != -1)
	{
		// "."과 ".." 제외
		if (!strcmp(fd.name, ".") || !strcmp(fd.name, ".."))
		{
			iResult = _findnext(handle, &fd);
			continue;
		}

		// 전체 경로 생성
		char szFullPath[256] = "";
		sprintf_s(szFullPath, "%s%s", pPath, fd.name);

		// 디렉터리인 경우 재귀 호출
		if (fd.attrib & _A_SUBDIR)
		{
			char szSubPath[256] = "";
			sprintf_s(szSubPath, "%s/", szFullPath);
			LoadSoundFileRecursive(szSubPath);
		}
		else // 파일인 경우
		{
			FMOD::Sound* pSound = nullptr;
			FMOD_RESULT result = m_pSystem->createSound(szFullPath, FMOD_DEFAULT, nullptr, &pSound);

			if (result == FMOD_OK)
			{
				// 파일명만 키로 사용 (경로 제외)
				wstring strSoundKey;

#ifdef _UNICODE
				int iLength = MultiByteToWideChar(CP_ACP, 0, fd.name, -1, nullptr, 0);
				strSoundKey.resize(iLength - 1);
				MultiByteToWideChar(CP_ACP, 0, fd.name, -1, &strSoundKey[0], iLength);
#else
				strSoundKey = fd.name;
#endif

				m_mapSound.emplace(strSoundKey, pSound);

				if (result == FMOD_OK)
				{
					// 로드 성공 로그
					OutputDebugStringA("사운드 로드 성공: ");
					OutputDebugStringA(fd.name);
					OutputDebugStringA("\n");
				}
			}
		}

		iResult = _findnext(handle, &fd);
	}

	_findclose(handle);
	return S_OK;
}

void CSound_Manager::Fade_BGM_Channel(FMOD::Channel* pChannel, float fFrom, float fTo, float fSeconds, bool bStopAtEnd)
{
	if (!pChannel)
		return;

	FMOD::System* pSystem = nullptr;
	if (pChannel->getSystemObject(&pSystem) != FMOD_OK || !pSystem)
		return;

	int iSampleRate = 48000;
	if (pSystem->getSoftwareFormat(
		&iSampleRate, nullptr, nullptr) != FMOD_OK)
		return;

	unsigned long long iDspClock = 0;
	if (pChannel->getDSPClock(nullptr, &iDspClock) != FMOD_OK)
		return;

	if (fSeconds < 0.f)
		fSeconds = 0.f;
	unsigned long long iFadeSamples =
		(unsigned long long)((double)iSampleRate * fSeconds);

	pChannel->removeFadePoints(0, ~0ull);
	pChannel->addFadePoint(iDspClock, fFrom);
	pChannel->addFadePoint(iDspClock + iFadeSamples, fTo);
	if (bStopAtEnd)
		pChannel->setDelay(0, iDspClock + iFadeSamples, true);
}

CSound_Manager* CSound_Manager::Create()
{
	CSound_Manager* pInstance = new CSound_Manager();
	if (nullptr == pInstance)
		return nullptr;

	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CSound_Manager::Free()
{
	__super::Free();

	StopAll();

	for (auto& pair : m_mapSound)
	{
		if (pair.second) { pair.second->release(); pair.second = nullptr; }
	}
	m_mapSound.clear();

	for (_uint i = 0; i < ETOUI(ESoundBus::END); ++i)
	{
		if (m_pBuses[i]) { m_pBuses[i]->release(); m_pBuses[i] = nullptr; }
	}

	if (m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}
