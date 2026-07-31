#pragma once
#include "Engine_Defines.h"
#include "Base.h"
#include "Sound_Handle.h"

namespace FMOD
{
    class Sound;
    class Channel;
    class ChannelGroup;
    class System;
    class DSP;
}

NS_BEGIN(Engine)

class CSound_Manager : public CBase
{
private:
    CSound_Manager();
    virtual ~CSound_Manager() = default;

private:
    struct SOUND_GATE
    {
        unsigned long long  ullLastMs = { 0 };
        unsigned int        iBurst = 0;
    };

public:
    void Update();

public:
    // 원샷 SFX: 채널 지정 없음. FMOD가 빈 채널 할당 + 핸들 반환(fire-and-forget면 무시)
    CSound_Handle PlaySFX(const TCHAR* pSoundKey, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
    CSound_Handle PlaySFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);

    // Loop SFX : 상태 코드에서 재생 필요할 때 사용
    CSound_Handle PlaySFXLoop(const TCHAR* pSoundKey, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
    CSound_Handle PlaySFX3DLoop(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
    CSound_Handle PlaySFXSectionLoop(const TCHAR* pSoundKey, float _fLoopStart, float fLoopEnd, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);

    // BGM: 소유 핸들로 교체(기존 정지 후 재생)
    void PlayBGM(const TCHAR* pSoundKey, float fVolume = 1.f, bool bLoop = true);
    void StopBGM();

    // 구간 반복 BGM: 0부터 재생, 이후 fStart01~fEnd01(0~1) 구간 반복
    void PlayBGM_Section(const TCHAR* pSoundKey, float fStart01, float fEnd01, float fVolume = 1.f);
    void Fade_BGM_Out(_float fSec);
    void Play_BGM_Fade(const TCHAR* pSoundKey, float fSec, float fVolume = 1.f, CSound_Handle* pOut = nullptr);
    void Resume_BGM_Fade(CSound_Handle& hBgm, float fInSec);

    // 버스(카테고리) 단위 제어
    void SetBusVolume(ESoundBus eBus, float fVolume);
    void StopBus(ESoundBus eBus);
    void StopAll();

    void Set_ListenerPos(const _float3& v) { m_vListenerPos = v; }
    void Set_ListenerPos(const _float4& v) { m_vListenerPos = { v.x, v.y, v.z }; }

    _wstring Get_Current_BGM_Key() const;

private:
    FMOD::Channel* PlayInternal(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus, bool bLoop);
    FMOD::Sound* Find_Sound(const TCHAR* pSoundKey) const;

    _bool Gate_Sound(FMOD::Sound* pSound, _float& fVolume);

private:
    std::map<std::wstring, FMOD::Sound*>    m_mapSound;
    std::map<FMOD::Sound*, SOUND_GATE>      m_mapSoundGate;

    FMOD::System* m_pSystem = nullptr;
    FMOD::DSP*  m_pLimiter = { nullptr };

    FMOD::ChannelGroup* m_pBuses[ETOUI(ESoundBus::END)] = {};
    FMOD::Channel* m_pBGMChannel = nullptr;   // BGM 소유 핸들

    _float3 m_vListenerPos = {};
    static constexpr _float m_fMaxDistance = 50.f;

    static constexpr unsigned long long     SOUND_GATE_WINDOW_MS = 60;
    static constexpr _uint                  SOUND_GATE_MAX_STACK = 3;
    static constexpr _float                 SOUND_GATE_FALLOFF = 0.6f;

private:
    HRESULT Initialize();
    HRESULT Ready_Buses();
    HRESULT LoadSoundFile();
    HRESULT LoadSoundFileRecursive(const char* pPath);

    static void Fade_BGM_Channel(FMOD::Channel* pChannel, float fFrom, float fTo, float fSeconds, bool bStopAtEnd);

public:
    static CSound_Manager* Create();
    virtual void Free() override;
};

NS_END