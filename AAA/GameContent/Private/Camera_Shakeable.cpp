#include "Camera_Shakeable.h"
#include "GameInstance.h"
#include "GameContent_Events.h"

void CCamera_Shakeable::Set_Active(_bool b)
{
    if (!b && m_bActive)
    {
        m_fZoomFovAdd = 0.f;
        m_fZoomDolly = 0.f;
        m_fZoomBlur = 0.f;
        Reset_ZoomPunch();
    }
    __super::Set_Active(b);
}

void CCamera_Shakeable::Tick_Shake(_float fTimeDelta)
{
    const _float fRaw = m_pGameInstance_Proxy->Get_RawTimeDelta(TEXT("Timer_60"));

    const _float fShakeDelta = m_bShakeRawTime ? fRaw : fTimeDelta;
    if (m_fTrauma > 0.f)
        m_fTrauma = max(0.f, m_fTrauma - m_fTraumaDecay * fShakeDelta);
    m_fShakeTime += fShakeDelta;

    // 줌은 셰이크와 시간축을 따로 쓴다(슬로모 중에도 스냅감 유지)
    Tick_ZoomPunch(m_bZoomRawTime ? fRaw : fTimeDelta);
}

void CCamera_Shakeable::Add_ZoomPunch(_float fFovAdd, _float fDolly, _float fBlur,
    _float fInDur, _float fHoldDur, _float fOutDur, _bool bIgnoreTimeScale)
{
    if (!m_bActive)
        return;

    // 재생 중인 펀치가 더 세면 무시(약한 펀치가 덮어써서 튀는 것 방지)
    const _float fCurFov = m_fZoomFovAdd * m_fZoomWeight;
    if (m_bZoomActive && fabsf(fCurFov) >= fabsf(fFovAdd))
        return;

    // 현재 weight를 유지한 지점에서 In 구간을 다시 타서 팝을 없앤다
    const _float fStartW = (fabsf(fFovAdd) > 1e-4f) ? min(1.f, fabsf(fCurFov / fFovAdd)) : 0.f;

    m_fZoomFovAdd = fFovAdd;
    m_fZoomDolly = fDolly;
    m_fZoomBlur = fBlur;
    m_fZoomInDur = max(0.f, fInDur);
    m_fZoomHoldDur = max(0.f, fHoldDur);
    m_fZoomOutDur = max(0.f, fOutDur);
    m_bZoomRawTime = bIgnoreTimeScale;

    // In 커브 1-(1-u)^3 의 역함수
    const _float u = 1.f - powf(max(0.f, 1.f - fStartW), 1.f / 3.f);
    m_fZoomTime = m_fZoomInDur * u;
    m_fZoomWeight = fStartW;
    m_bZoomActive = true;
}

void CCamera_Shakeable::Tick_ZoomPunch(_float fTimeDelta)
{
    if (!m_bZoomActive)
        return;

    m_fZoomTime += fTimeDelta;

    const _float fTotal = m_fZoomInDur + m_fZoomHoldDur + m_fZoomOutDur;
    if (m_fZoomTime >= fTotal)
    {
        m_bZoomActive = false;
        m_fZoomWeight = 0.f;
        Push_RadialBlur(0.f);
        return;
    }

    m_fZoomWeight = Calc_ZoomWeight(m_fZoomTime);
    Push_RadialBlur(m_fZoomBlur * m_fZoomWeight);
}

_float CCamera_Shakeable::Calc_ZoomWeight(_float t) const
{
    if (t < m_fZoomInDur)
    {
        const _float u = (m_fZoomInDur > 0.f) ? (t / m_fZoomInDur) : 1.f;
        const _float inv = 1.f - u;
        return 1.f - inv * inv * inv;              // ease-out: 확 빠지고 감속
    }
    t -= m_fZoomInDur;

    if (t < m_fZoomHoldDur)
        return 1.f;
    t -= m_fZoomHoldDur;

    const _float u = (m_fZoomOutDur > 0.f) ? min(1.f, t / m_fZoomOutDur) : 1.f;
    return 1.f - u * u * (3.f - 2.f * u);          // smoothstep 복귀
}

void CCamera_Shakeable::Push_RadialBlur(_float fAmount)
{
    // 한 번도 안 건드린 상태면 전역값(에디터 튜닝값)을 그대로 둔다
    if (fAmount <= 0.f && !m_bBlurPushed)
        return;

    m_pGameInstance_Proxy->Set_ShaderGlobal("g_fRadialBlur", _float4(fAmount, 0.f, 0.f, 0.f));
    m_bBlurPushed = (fAmount > 0.f);
}

void CCamera_Shakeable::Apply_ZoomFov()
{
    if (m_fZoomWeight > 0.f)
    {
        m_fFovy += XMConvertToRadians(m_fZoomFovAdd * m_fZoomWeight);
        m_fFovy = max(XMConvertToRadians(5.f), min(XMConvertToRadians(150.f), m_fFovy));
    }
    Recalculate_ProjMatrix();
}

void CCamera_Shakeable::Apply_Shake(_matrix& CamWorld)
{
    _vector eye = CamWorld.r[3];

    // 줌 펀치 dolly: 회전/주시점은 그대로 두고 시선 반대로만 물러난다
    if (m_fZoomWeight > 0.f && fabsf(m_fZoomDolly) > 1e-5f)
        eye = XMVectorSubtract(eye, XMVectorScale(CamWorld.r[2], m_fZoomDolly * m_fZoomWeight));

    const _float a = min(1.f, m_fTrauma + m_fRumble);
    if (a <= 0.f)
    {
        CamWorld.r[3] = XMVectorSetW(eye, 1.f);
        return;
    }

    const _float s = a * a;
    const _float t = m_fShakeTime * m_fShakeFreq;

    const _float yaw = m_fShakeYaw * s * sinf(t);
    const _float pitch = m_fShakePitch * s * sinf(t * 1.27f + 7.f);
    const _float roll = m_fShakeRoll * s * sinf(t * 1.63f + 19.f);

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

    Subscribe_Event(EventTag::Camera_ZoomPunch, [this](void* p) {
        if (auto* d = static_cast<CAMERA_ZOOMPUNCH_DESC*>(p))
            Add_ZoomPunch(d->fFovAdd, d->fDolly, d->fBlur,
                d->fInDur, d->fHoldDur, d->fOutDur, d->bIgnoreTimeScale);
        else
            Add_ZoomPunch(7.f, 1.5f);
        });

    return S_OK;
}