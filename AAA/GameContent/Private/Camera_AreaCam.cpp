#include "Camera_AreaCam.h"
#include "GameInstance.h"

static _vector SmoothDampV(_fvector cur, _fvector target, _vector& vel, _float smoothTime, _float dt)
{
    smoothTime = max(0.0001f, smoothTime);
    _float omega = 2.f / smoothTime, x = omega * dt;
    _float e = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
    _vector change = XMVectorSubtract(cur, target);
    _vector temp = XMVectorScale(XMVectorAdd(vel, XMVectorScale(change, omega)), dt);
    vel = XMVectorScale(XMVectorSubtract(vel, XMVectorScale(temp, omega)), e);
    return XMVectorAdd(target, XMVectorScale(XMVectorAdd(change, temp), e));
}

CCamera_AreaCam::CCamera_AreaCam(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : CCamera_Shakeable(pDevice, pContext)
{
}
CCamera_AreaCam::CCamera_AreaCam(const CCamera_AreaCam& Prototype) : CCamera_Shakeable(Prototype) {}

HRESULT CCamera_AreaCam::Initialize_Prototype() { return S_OK; }

HRESULT CCamera_AreaCam::Initialize(void* pArg)
{
    if (auto pDesc = static_cast<AREACAM_DESC*>(pArg)) {
        m_strDataPath = pDesc->strDataPath;
    }
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;

    m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(1.f, 0.f, 0.f, 0.f));
    m_pTransformCom->Set_State(STATE::UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));
    m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 3.f, -10.f, 1.f));
    m_pTransformCom->LookAt(XMVectorSet(0.f, 0.f, 0.f, 1.f));

    m_solver.Load(m_strDataPath);
    m_bActive = true;
    return S_OK;
}

void CCamera_AreaCam::Priority_Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);
    if (!m_bActive) { __super::Priority_Update(fTimeDelta); return; }
    Tick_Shake(fTimeDelta);
    if (!m_pTarget)
    {
        PLAYER_QUERY p;
        m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &p);
        m_pTarget = p.pPlayer;
    }

    _vector vKirby = m_pTarget ? m_pTarget->Get_Transform()->Get_State(STATE::POSITION) : XMVectorSet(0.f, 0.f, 0.f,
        1.f);

    if (m_solver.Cur_GazeMode() == 2) {
        string tag = m_solver.Cur_GazeTag();
        CGameObject* pTarget = nullptr;
        if (pTarget) m_solver.Set_GazeOverride(pTarget->Get_Transform()->Get_State(STATE::POSITION), true);
        else         m_solver.Set_GazeOverride(XMVectorZero(), false);
    }
    else m_solver.Set_GazeOverride(XMVectorZero(), false);

    m_solver.Update(vKirby, fTimeDelta);

    _int curArea = m_solver.Cur_AreaIndex();
    if (m_bInit && curArea >= 0 && m_lastArea >= 0 && curArea != m_lastArea)
        m_blendTimer = m_blendDur;                 // 전환 시작
    m_lastArea = curArea;

    _float fBlend = (m_blendDur > 0.f) ? (m_blendTimer / m_blendDur) : 0.f; // 1→0
    _float fEase = fBlend * fBlend * (3.f - 2.f * fBlend);                 // smoothstep
    _float smoothT = m_smoothBase + (m_smoothTrans - m_smoothBase) * fEase;
    if (m_blendTimer > 0.f) m_blendTimer = max(0.f, m_blendTimer - fTimeDelta);

    const CAM_POSE& pose = m_solver.Cur_Pose();

    _vector vEye = XMLoadFloat3(&pose.eye);
    _vector vFwd = XMLoadFloat3(&pose.fwd);

    // NaN/degenerate 차단 (NaN은 부등호로 안 걸러져서 IsNaN 명시).
    // 한 프레임 NaN이 LookAt로 transform에 영구전염되는 것 방지.
    _bool bBad = XMVector3IsNaN(vEye) || XMVector3IsNaN(vFwd)
        || (XMVectorGetX(XMVector3LengthSq(vFwd)) < 1e-6f);
    if (bBad)
    {
        vEye = m_bInit ? XMLoadFloat3(&m_eyeCur) : XMVectorSet(0.f, 3.f, -10.f, 0.f);
        vFwd = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        XMStoreFloat3(&m_eyeVel, XMVectorZero());   // 속도 NaN 전파도 차단
        XMStoreFloat3(&m_atVel, XMVectorZero());
    }
    _vector vAt = XMVectorAdd(vEye, vFwd);

    // ===== 커비 변신 줌인 =====
    // 목표 weight를 향해 램프(줌인/복귀 시간 분리) 후 smoothstep 이징.
    _float wTarget = m_bTransZoom ? 1.f : 0.f;
    _float wRate = m_bTransZoom
        ? (m_transInDur > 0.f ? 1.f / m_transInDur : 1e9f)
        : (m_transOutDur > 0.f ? 1.f / m_transOutDur : 1e9f);
    if (m_transWeight < wTarget) m_transWeight = min(wTarget, m_transWeight + wRate * fTimeDelta);
    else                         m_transWeight = max(wTarget, m_transWeight - wRate * fTimeDelta);
    _float wEase = m_transWeight * m_transWeight * (3.f - 2.f * m_transWeight); // smoothstep

    _float fFovDeg = pose.fov;
    if (wEase > 0.f && m_pTarget)
    {
        // position(eye)은 그대로 두고 시선(at)만 커비로 회전 + fov 축소.
        _vector vAim = XMVectorAdd(vKirby, XMVectorSet(0.f, m_transAimY, 0.f, 0.f));
        vAim = XMVectorSetW(vAim, 1.f);
        vAt = XMVectorLerp(vAt, vAim, wEase);
        fFovDeg = pose.fov + (m_transFov - pose.fov) * wEase;
    }
    // ==========================

    if (!m_bInit) { XMStoreFloat3(&m_eyeCur, vEye); XMStoreFloat3(&m_atCur, vAt); m_bInit = true; }

    _vector eVel = XMLoadFloat3(&m_eyeVel), aVel = XMLoadFloat3(&m_atVel);
    XMStoreFloat3(&m_eyeCur, SmoothDampV(XMLoadFloat3(&m_eyeCur), vEye, eVel, smoothT, fTimeDelta));
    XMStoreFloat3(&m_atCur, SmoothDampV(XMLoadFloat3(&m_atCur), vAt, aVel, smoothT, fTimeDelta));
    XMStoreFloat3(&m_eyeVel, eVel); XMStoreFloat3(&m_atVel, aVel);

    if (m_fFovCurDeg < 0.f)
        m_fFovCurDeg = fFovDeg;
    m_fFovCurDeg += (fFovDeg - m_fFovCurDeg) * min(1.f, fTimeDelta * 5.f);
    m_fFovy = XMConvertToRadians(m_fFovCurDeg);
    Recalculate_ProjMatrix();

    _vector vE = XMLoadFloat3(&m_eyeCur);
    _vector vDir = XMVectorSubtract(XMLoadFloat3(&m_atCur), vE);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-6f)   // 눈==타깃 방어
        vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    _vector vLook = XMVector3Normalize(vDir);
    _vector vUpRef = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Cross(vUpRef, vLook);
    if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-6f)  // fwd 수직(평행) 방어
        vRight = XMVector3Cross(XMVectorSet(0.f, 0.f, 1.f, 0.f), vLook);
    vRight = XMVector3Normalize(vRight);
    _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    _matrix CamWorld;
    CamWorld.r[0] = XMVectorSetW(vRight, 0.f);
    CamWorld.r[1] = XMVectorSetW(vUp, 0.f);
    CamWorld.r[2] = XMVectorSetW(vLook, 0.f);
    CamWorld.r[3] = XMVectorSetW(vE, 1.f);
    Apply_Shake(CamWorld);

    m_pTransformCom->Set_State(STATE::RIGHT, CamWorld.r[0]);
    m_pTransformCom->Set_State(STATE::UP, CamWorld.r[1]);
    m_pTransformCom->Set_State(STATE::LOOK, CamWorld.r[2]);
    m_pTransformCom->Set_State(STATE::POSITION, CamWorld.r[3]);

    __super::Priority_Update(fTimeDelta);
}

HRESULT CCamera_AreaCam::Ready_Events()
{
    Subscribe_Event(EventTag::Kirby_Ability_Changed, [this](void* pData)
        {
            _bool bBegin = pData ? static_cast<KIRBY_ABILITY_CHANGED*>(pData)->bBegin : true;
            m_bTransZoom = bBegin;   // Priority_Update에서 weight가 알아서 램프/복귀

            if (bBegin)
                m_pGameInstance_Proxy->Tween_ShaderGlobal("g_fSpotlightDarken", s_vSpotlightDarken, 0.5f);
            else
                m_pGameInstance_Proxy->Tween_ShaderGlobal("g_fSpotlightDarken", _float4(0.f, 0.f, 0.f, 0.f), 0.5f);
        });

    return Ready_ShakeEvents();
}

_float CCamera_AreaCam::Resolve_TimeDelta(_float fTimeDelta)
{
    if (m_bTransZoom)
        return m_pGameInstance_Proxy->Get_RawTimeDelta(L"Timer_60");
    else
        return fTimeDelta;
}

CCamera_AreaCam* CCamera_AreaCam::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCamera_AreaCam* p = new CCamera_AreaCam(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CCamera_AreaCam"); Safe_Release(p); }
    return p;
}
CGameObject* CCamera_AreaCam::Clone(void* pArg)
{
    CCamera_AreaCam* p = new CCamera_AreaCam(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CCamera_AreaCam"); Safe_Release(p); }
    return p;
}
void CCamera_AreaCam::Free() { __super::Free(); }