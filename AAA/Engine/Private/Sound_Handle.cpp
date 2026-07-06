#include "Sound_Handle.h"
#include "fmod/fmod.hpp"

CSound_Handle::CSound_Handle(FMOD::Channel* pChannel)
    : m_pChannel(pChannel)
{
}

void CSound_Handle::Stop()
{
    if (!m_pChannel)
        return;

    bool bPlaying = false;
    if (m_pChannel->isPlaying(&bPlaying) == FMOD_OK && bPlaying)
        m_pChannel->stop();

    m_pChannel = nullptr;
}

void CSound_Handle::Release_Loop()
{
    if (!m_pChannel)
        return;

    FMOD_MODE mode = 0;
    if (m_pChannel->getMode(&mode) != FMOD_OK)
        return;

    mode &= ~(FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI);
    mode |= FMOD_LOOP_OFF;
    m_pChannel->setMode(mode);
}

void CSound_Handle::Set_Paused(_bool bPause)
{
    if (!m_pChannel)
        return;

    m_pChannel->setPaused(bPause);
}

void CSound_Handle::Set_Volume(_float fVolume)
{
    if (!m_pChannel)
        return;

    m_pChannel->setVolume(fVolume);
}

_bool CSound_Handle::Is_Playing() const
{
    if (!m_pChannel)
        return false;

    bool bPlaying = false;
    if (m_pChannel->isPlaying(&bPlaying) != FMOD_OK)
        return false;

    return bPlaying;
}
