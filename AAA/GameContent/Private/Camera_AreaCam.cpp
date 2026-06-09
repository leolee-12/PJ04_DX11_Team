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

    CAM_POSE pose = m_solver.Solve(vKirby);
    _vector vEye = XMLoadFloat3(&pose.eye);
    _vector vAt = XMVectorAdd(vEye, XMLoadFloat3(&pose.fwd));

#ifdef _DEBUG
    {
        _float3 k; XMStoreFloat3(&k, vKirby);
        char b[224];
        sprintf_s(b, "K(%.1f,%.1f,%.1f) area=%d rail=%d gaze=%d | EYE(%.1f,%.1f,%.1f) FWD(%.2f,%.2f,%.2f)\n",
            k.x, k.y, k.z, m_solver.Cur_AreaIndex(), (int)m_solver.Cur_UseRail(), (int)m_solver.Cur_Gazing(),
            pose.eye.x, pose.eye.y, pose.eye.z, pose.fwd.x, pose.fwd.y, pose.fwd.z);
        OutputDebugStringA(b);
    }
#endif

    if (XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(vEye, vAt))) < 1e-4f)
        vAt = XMVectorAdd(vEye, XMVectorSet(0.f, 0.f, 1.f, 0.f));

    if (!m_bInit) { XMStoreFloat3(&m_eyeCur, vEye); XMStoreFloat3(&m_atCur, vAt); m_bInit = true; }

    _vector eVel = XMLoadFloat3(&m_eyeVel), aVel = XMLoadFloat3(&m_atVel);
    XMStoreFloat3(&m_eyeCur, SmoothDampV(XMLoadFloat3(&m_eyeCur), vEye, eVel, m_smoothTime, fTimeDelta));
    XMStoreFloat3(&m_atCur, SmoothDampV(XMLoadFloat3(&m_atCur), vAt, aVel, m_smoothTime, fTimeDelta));
    XMStoreFloat3(&m_eyeVel, eVel); XMStoreFloat3(&m_atVel, aVel);

    m_fFovy = XMConvertToRadians(pose.fov);
    Recalculate_ProjMatrix();
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_eyeCur), 1.f));
    m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&m_atCur), 1.f));

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