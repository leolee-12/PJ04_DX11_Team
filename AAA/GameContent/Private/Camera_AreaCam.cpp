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

CCamera_AreaCam::CCamera_AreaCam(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) : CCamera{ pDevice, pContext }
{
}
CCamera_AreaCam::CCamera_AreaCam(const CCamera_AreaCam& Prototype) : CCamera(Prototype) {}

HRESULT CCamera_AreaCam::Initialize_Prototype() { return S_OK; }

HRESULT CCamera_AreaCam::Initialize(void* pArg)
{
    if (auto pDesc = static_cast<AREACAM_DESC*>(pArg)) {
        m_strTargetLayer = pDesc->strTargetLayer;
        m_strTargetObj = pDesc->strTargetObj;
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
    if (!m_bActive) { __super::Priority_Update(fTimeDelta); return; }
    if (!m_pTarget)
        m_pTarget = m_pGameInstance_Proxy->Find_GameObject(Get_LevelIndex(), m_strTargetLayer, m_strTargetObj);
    _vector vKirby = m_pTarget ? m_pTarget->Get_Transform()->Get_State(STATE::POSITION) : XMVectorSet(0.f, 0.f, 0.f,
        1.f);

    if (m_solver.Cur_GazeMode() == 2) {
        string tag = m_solver.Cur_GazeTag();
        // TODO: 프로젝트의 보스/타깃 찾기 방식에 맞춰 위치 획득.
        //   예) wstring wtag(tag.begin(), tag.end());
        //       CGameObject* p = m_pGameInstance_Proxy->Find_GameObject(Get_LevelIndex(), L"Layer_Boss", wtag);
        CGameObject* pTarget = nullptr;   // <- 위 Find_GameObject로 교체
        if (pTarget) m_solver.Set_GazeOverride(pTarget->Get_Transform()->Get_State(STATE::POSITION), true);
        else         m_solver.Set_GazeOverride(XMVectorZero(), false);
    }
    else m_solver.Set_GazeOverride(XMVectorZero(), false);

    m_solver.Update(vKirby, fTimeDelta);
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

    if (!m_bInit) { XMStoreFloat3(&m_eyeCur, vEye); XMStoreFloat3(&m_atCur, vAt); m_bInit = true; }

    _vector eVel = XMLoadFloat3(&m_eyeVel), aVel = XMLoadFloat3(&m_atVel);
    XMStoreFloat3(&m_eyeCur, SmoothDampV(XMLoadFloat3(&m_eyeCur), vEye, eVel, m_smoothTime, fTimeDelta));
    XMStoreFloat3(&m_atCur, SmoothDampV(XMLoadFloat3(&m_atCur), vAt, aVel, m_smoothTime, fTimeDelta));
    XMStoreFloat3(&m_eyeVel, eVel); XMStoreFloat3(&m_atVel, aVel);

    m_fFovy = XMConvertToRadians(pose.fov);
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

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vE, 1.f));
    m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSetW(vRight, 0.f));
    m_pTransformCom->Set_State(STATE::UP, XMVectorSetW(vUp, 0.f));
    m_pTransformCom->Set_State(STATE::LOOK, XMVectorSetW(vLook, 0.f));

    __super::Priority_Update(fTimeDelta);
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