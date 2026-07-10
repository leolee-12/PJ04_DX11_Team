#pragma once
#include "Engine_Defines.h"

namespace FMOD { class Channel; }

NS_BEGIN(Engine)

// Value-type wrapper for a playing sound channel.
// Copyable; all ops are safe on null/expired handles (FMOD 2.x
// generation-checked). Stop() also resets to null. No auto-stop
// in destructor by design.
class ENGINE_DLL CSound_Handle final
{
    friend class CSound_Manager;
public:
    CSound_Handle() = default;
    explicit CSound_Handle(FMOD::Channel* pChannel);

    void    Stop();

    // 루프 모드를 껐을 때 사운드 끝(1.0)까지 끝까지 재생하고 종료하고 싶을 때 사용
    void    Release_Loop();             

    void    Set_Paused(_bool bPause);
    void    Set_Volume(_float fVolume);

    _bool   Is_Playing() const;
    _bool   Is_Valid() const { return nullptr != m_pChannel; }

private:
    FMOD::Channel* m_pChannel = { nullptr };
};

NS_END
