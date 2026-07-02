#pragma once
#include "Engine_Defines.h"
#include "Base.h"

namespace FMOD
{
    class Sound;
    class Channel;
    class ChannelGroup;
    class System;
}

NS_BEGIN(Engine)

class CSound_Manager : public CBase
{
private:
    CSound_Manager();
    virtual ~CSound_Manager() = default;

public:
    void Update();

public:
    // 원샷 SFX: 채널 지정 없음. FMOD가 빈 채널 할당 + 핸들 반환(fire-and-forget면 무시)
    FMOD::Channel* PlaySFX(const TCHAR* pSoundKey, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
    FMOD::Channel* PlaySFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);

    // BGM: 소유 핸들로 교체(기존 정지 후 재생)
    void PlayBGM(const TCHAR* pSoundKey, float fVolume = 1.f, bool bLoop = true);
    void StopBGM();

    // 버스(카테고리) 단위 제어
    void SetBusVolume(ESoundBus eBus, float fVolume);
    void StopBus(ESoundBus eBus);
    void StopAll();

    void Set_ListenerPos(const _float3& v) { m_vListenerPos = v; }
    void Set_ListenerPos(const _float4& v) { m_vListenerPos = { v.x, v.y, v.z }; }

private:
    FMOD::Channel* PlayInternal(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus, bool bLoop);
    FMOD::Sound* Find_Sound(const TCHAR* pSoundKey) const;

private:
    std::map<std::wstring, FMOD::Sound*> m_mapSound;
    FMOD::System* m_pSystem = nullptr;

    FMOD::ChannelGroup* m_pBuses[ETOUI(ESoundBus::END)] = {};
    FMOD::Channel* m_pBGMChannel = nullptr;   // BGM 소유 핸들

    _float3 m_vListenerPos = {};
    static constexpr _float m_fMaxDistance = 20.f;

private:
    HRESULT Initialize();
    HRESULT Ready_Buses();
    HRESULT LoadSoundFile();
    HRESULT LoadSoundFileRecursive(const char* pPath);

public:
    static CSound_Manager* Create();
    virtual void Free() override;
};

NS_END