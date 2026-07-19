#include "Camera_Shakeable.h"
#include "GameInstance.h"
#include "GameContent_Events.h"

void CCamera_Shakeable::Tick_Shake(_float fTimeDelta)
{
    if (m_bShakeRawTime)
        fTimeDelta = m_pGameInstance_Proxy->Get_RawTimeDelta(TEXT("Timer_60"));

    if (m_fTrauma > 0.f)
        m_fTrauma = max(0.f, m_fTrauma - m_fTraumaDecay * fTimeDelta);
    m_fShakeTime += fTimeDelta;
}

void CCamera_Shakeable::Apply_Shake(_matrix& CamWorld)
{
    _float a = min(1.f, m_fTrauma + m_fRumble);
    if (a <= 0.f) return;

    _float s = a * a;
    _float t = m_fShakeTime * m_fShakeFreq;

    _float yaw = m_fShakeYaw * s * sinf(t);
    _float pitch = m_fShakePitch * s * sinf(t * 1.27f + 7.f);
    _float roll = m_fShakeRoll * s * sinf(t * 1.63f + 19.f);

    _vector eye = CamWorld.r[3];
    _vector right = CamWorld.r[0];
    _vector up = CamWorld.r[1];

    _matrix Rot = CamWorld; Rot.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
    _matrix NewRot = XMMatrixRotationRollPitchYaw(pitch, yaw, roll) * Rot;

    _vector posJitter = right * (m_fShakePos * s * sinf(t * 1.11f + 3.f))
        + up * (m_fShakePos * s * sinf(t * 1.39f + 13.f));

    CamWorld.r[0] = NewRot.r[0];
    CamWorld.r[1] = NewRot.r[1];
    CamWorld.r[2] = NewRot.r[2];
    CamWorld.r[3] = XMVectorSetW(eye + posJitter, 1.f);
}

HRESULT CCamera_Shakeable::Ready_ShakeEvents()
{
    Subscribe_Event(EventTag::Camera_Shake, [this](void* p) {
        if (auto* d = static_cast<CAMERA_SHAKE_DESC*>(p))
            Add_Shake(d->fTrauma, d->fDuration, d->bIgnoreTimeScale);
        else
            Add_Shake(0.5f);
        });

    Subscribe_Event(EventTag::Camera_Rumble, [this](void* p) {
        _float lvl = p ? *static_cast<_float*>(p) : 0.f;
        if (lvl > 0.f) Set_Rumble(lvl);
        else           Stop_Rumble();
        });
    return S_OK;
}