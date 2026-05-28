#pragma once
#include "Engine_Defines.h"
#include "Base.h"

// FMOD 전방 선언
namespace FMOD
{
	class Sound;
	class Channel;
	class System;
}

NS_BEGIN(Engine)

class CSound_Manager : public CBase
{
private:
	CSound_Manager();
	virtual ~CSound_Manager() = default;
public:
	void Update(); // FMOD 시스템 업데이트 (매 프레임 호출 필요)
public:
	void Play(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume);
	void PlaySound3D(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume, _fvector vSoundPos);
	void PlayBGM(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume);
	void StopSound(_uint iChannelIndex);
	void StopAll();
	void SetChannelVolume(_uint iChannelIndex, float fVolume);

	void Set_ListenerPos(const _float3& vListnerPos) { m_vListenerPos = vListnerPos; }
	void Set_ListenerPos(const _float4& vListnerPos) { 
		m_vListenerPos.x = vListnerPos.x; m_vListenerPos.y = vListnerPos.y; m_vListenerPos.z = vListnerPos.z;
	}
private:
	// 사운드 리소스 정보를 갖는 객체 (64비트 FMOD::Sound 포인터)
	std::map<std::wstring, FMOD::Sound*> m_mapSound;

	// FMOD::Channel : 재생하고 있는 사운드를 관리할 객체
	FMOD::Channel* m_pChannels[SOUND_MAX_CHANNEL];

	// 사운드, 채널 객체 및 장치를 관리하는 객체 (64비트 FMOD::System)
	FMOD::System* m_pSystem;

	_float3		  m_vListenerPos;
	static constexpr _float	m_fMaxDistance = 20.f;

private:
	HRESULT Initialize();
	HRESULT LoadSoundFile();
	HRESULT LoadSoundFileRecursive(const char* pPath);

public:
	static CSound_Manager* Create();
	virtual void Free() override;
};

NS_END
