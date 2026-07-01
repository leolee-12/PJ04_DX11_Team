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
	static const char* s_szBusName[] = { "BGM", "SFX", "UI", "VOICE" };

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

FMOD::Channel* CSound_Manager::PlaySFX(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus)
{
	return PlayInternal(pSoundKey, fVolume, eBus, false);
}

FMOD::Channel* CSound_Manager::PlaySFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume, ESoundBus eBus)
{
	_vector vDir = vSoundPos - XMLoadFloat3(&m_vListenerPos);
	_float fDistance = XMVectorGetX(XMVector3Length(vDir));

	_float fAtten = 1.f - (fDistance / m_fMaxDistance);
	fAtten = max(0.f, fAtten);
	if (fAtten < 0.001f)
		return nullptr;

	return PlayInternal(pSoundKey, fVolume * fAtten, eBus, false);
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
