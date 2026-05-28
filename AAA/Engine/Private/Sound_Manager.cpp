#include "Sound_Manager.h"
#include "fmod/fmod.hpp"
#include <io.h>

CSound_Manager::CSound_Manager()
	: m_pSystem(nullptr)
{
	ZeroMemory(m_pChannels, sizeof(m_pChannels));
}

HRESULT CSound_Manager::Initialize()
{
	FMOD_RESULT result;

	// FMOD 시스템 생성
	result = FMOD::System_Create(&m_pSystem);
	if (result != FMOD_OK)
	{
		// 에러 처리
		return E_FAIL;
	}

	// FMOD 시스템 초기화
	// 매개변수: (최대 채널 수, 초기화 플래그, 추가 드라이버 데이터)
	result = m_pSystem->init(SOUND_MAX_CHANNEL, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		// 에러 처리
		return E_FAIL;
	}

	// 사운드 파일 로드
	return LoadSoundFile();
}

void CSound_Manager::Update()
{
	// FMOD 시스템 업데이트 (매 프레임 호출 필요)
	if (m_pSystem)
	{
		m_pSystem->update();
	}
}

void CSound_Manager::Play(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume)
{
	if (iChannelIndex >= SOUND_MAX_CHANNEL) return;

	auto iter = m_mapSound.find(pSoundKey);

	if (iter == m_mapSound.end())
	{
		OutputDebugString(L"[사운드 매니저] 사운드를 찾을 수 없음: ");
		OutputDebugString(pSoundKey);
		OutputDebugString(L"\n");
		return;
	}

	OutputDebugString(L"[사운드 매니저] 재생 시도: ");
	OutputDebugString(pSoundKey);
	OutputDebugString(L"\n");

	bool bPlaying = false;
	if (m_pChannels[iChannelIndex])
	{
		m_pChannels[iChannelIndex]->isPlaying(&bPlaying);
	}

	// 해당 채널에서 재생 중이면 정지
	if (bPlaying)
	{
		m_pChannels[iChannelIndex]->stop();
	}

	// 사운드 재생
	FMOD_RESULT result = m_pSystem->playSound(iter->second, nullptr, true, &m_pChannels[iChannelIndex]);

	if (result == FMOD_OK)
	{
		OutputDebugString(L"[사운드 매니저] 재생 성공!\n");
		// 볼륨 설정 (0.0 ~ 1.0)
		m_pChannels[iChannelIndex]->setVolume(fVolume);
		m_pChannels[iChannelIndex]->setPaused(false);
	}
	else
	{
		OutputDebugString(L"[사운드 매니저] 재생 실패!\n");
	}
}

void CSound_Manager::PlaySound3D(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume, _fvector vSoundPos)
{
	if (iChannelIndex >= SOUND_MAX_CHANNEL) return;
	//if (vSoundPos.z < m_vListenerPos.z) { return; }

	_vector vDir = vSoundPos - XMLoadFloat3(&m_vListenerPos);
	_float fDistance = XMVectorGetX(XMVector3Length(vDir));

	_float fAttenuation = 1.f - (fDistance / m_fMaxDistance);
	fAttenuation = max(0.f, fAttenuation);

	if (fAttenuation < 0.001f) { return; }

	_float fFinalVolume = fVolume * fAttenuation;

	auto iter = m_mapSound.find(pSoundKey);

	if (iter == m_mapSound.end())
	{
		OutputDebugString(L"[사운드 매니저] 사운드를 찾을 수 없음: ");
		OutputDebugString(pSoundKey);
		OutputDebugString(L"\n");
		return;
	}

	OutputDebugString(L"[사운드 매니저] 재생 시도: ");
	OutputDebugString(pSoundKey);
	OutputDebugString(L"\n");

	bool bPlaying = false;
	if (m_pChannels[iChannelIndex])
	{
		m_pChannels[iChannelIndex]->isPlaying(&bPlaying);
	}

	// 해당 채널에서 재생 중이면 정지
	if (bPlaying)
	{
		m_pChannels[iChannelIndex]->stop();
	}

	// 사운드 재생
	FMOD_RESULT result = m_pSystem->playSound(iter->second, nullptr, true, &m_pChannels[iChannelIndex]);

	if (result == FMOD_OK)
	{
		OutputDebugString(L"[사운드 매니저] 재생 성공!\n");
		// 볼륨 설정 (0.0 ~ 1.0)
		m_pChannels[iChannelIndex]->setVolume(fFinalVolume);
		m_pChannels[iChannelIndex]->setPaused(false);
	}
	else
	{
		OutputDebugString(L"[사운드 매니저] 재생 실패!\n");
	}
}

void CSound_Manager::PlayBGM(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume)
{
	if (iChannelIndex >= SOUND_MAX_CHANNEL) return;

	auto iter = m_mapSound.find(pSoundKey);
	if (iter == m_mapSound.end()) return;

	bool bPlaying = false;
	if (m_pChannels[iChannelIndex])
	{
	    m_pChannels[iChannelIndex]->isPlaying(&bPlaying);
	    if (bPlaying)
	            m_pChannels[iChannelIndex]->stop();
	}

	FMOD_RESULT result = m_pSystem->playSound(iter->second, nullptr, true, &m_pChannels[iChannelIndex]);

	if (result == FMOD_OK)
	{
		OutputDebugString(L"[사운드 매니저] 재생 성공!\n");
		// 볼륨 설정 (0.0 ~ 1.0)
		m_pChannels[iChannelIndex]->setVolume(fVolume);
		m_pChannels[iChannelIndex]->setMode(FMOD_LOOP_NORMAL);
		m_pChannels[iChannelIndex]->setPaused(false);
	}
	else
	{
		OutputDebugString(L"[사운드 매니저] 재생 실패!\n");
	}
}

void CSound_Manager::StopSound(_uint iChannelIndex)
{
	if (m_pChannels[iChannelIndex])
	{
		m_pChannels[iChannelIndex]->stop();
	}
}

void CSound_Manager::StopAll()
{
	for (int i = 0; i < SOUND_MAX_CHANNEL; ++i)
	{
		if (m_pChannels[i])
		{
			m_pChannels[i]->stop();
		}
	}
}

void CSound_Manager::SetChannelVolume(_uint iChannelIndex, float fVolume)
{
	if (m_pChannels[iChannelIndex])
	{
		m_pChannels[iChannelIndex]->setVolume(fVolume);
	}
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
	// 모든 사운드 정지
	StopAll();

	// 사운드 리소스 해제
	for (auto& pair : m_mapSound)
	{
		if (pair.second)
		{
			pair.second->release();
			pair.second = nullptr;
		}
	}
	m_mapSound.clear();

	// FMOD 시스템 종료 및 해제
	if (m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}