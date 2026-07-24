#include "Camera_Coaster.h"
#include "GameInstance.h"

// Boss 카메라와 동일한 크리티컬리 댐프드 스무딩
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

CCamera_Coaster::CCamera_Coaster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCamera_Shakeable(pDevice, pContext) {
}
CCamera_Coaster::CCamera_Coaster(const CCamera_Coaster& Prototype)
    : CCamera_Shakeable(Prototype) {
}

HRESULT CCamera_Coaster::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    m_bActive = false;
    return S_OK;
}

void CCamera_Coaster::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) { __super::Priority_Update(fTimeDelta); return; }

    Tick_Shake(fTimeDelta);

    // 타깃(커비) 확보 - 이벤트 쿼리
    if (!m_pPlayer) { PLAYER_QUERY q{}; m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &q); m_pPlayer = q.pPlayer; }

    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector P = m_pPlayer ? m_pPlayer->Get_Transform()->Get_State(STATE::POSITION) : XMVectorSet(0, 0, 0, 1);

    // forward = 커비 look (요+피치 그대로, 롤 무시)
    _vector vFwd = m_pPlayer ? m_pPlayer->Get_Transform()->Get_State(STATE::LOOK) : XMVectorSet(0, 0, 1, 0);
    if (XMVectorGetX(XMVector3LengthSq(vFwd)) < 1e-6f) vFwd = XMVectorSet(0, 0, 1, 0);
    vFwd = XMVector3Normalize(vFwd);

    // 진행방향 기준 수평 right (요만 반영, 롤 없음)
    _vector vRight = XMVector3Cross(vUp, vFwd);
    if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-6f) vRight = XMVectorSet(1, 0, 0, 0);   // 수직 급상승/급강하 방어
    vRight = XMVector3Normalize(vRight);

    // 피치를 따라가는 코스터 up (상승 시 뒤+위, 하강 시 뒤+아래) - 롤은 없음
    _vector vUpCoaster = XMVector3Normalize(XMVector3Cross(vFwd, vRight));

    // eye: 커비에서 진행 반대로 물러나고 "코스터 up"으로 높임
    _vector vEye = XMVectorAdd(P, XMVectorScale(vUpCoaster, m_fHeight));
    vEye = XMVectorSubtract(vEye, XMVectorScale(vFwd, m_fBackDist));

    // at: 진행방향 앞 + 코스터 up 방향 시선 높이
    _vector vAt = XMVectorAdd(P, XMVectorScale(vFwd, m_fAimAhead));
    vAt = XMVectorAdd(vAt, XMVectorScale(vUpCoaster, m_fAimHeight));

    if (!m_bInit) { XMStoreFloat3(&m_eyeCur, vEye); XMStoreFloat3(&m_atCur, vAt); m_bInit = true; }

    _vector eVel = XMLoadFloat3(&m_eyeVel), aVel = XMLoadFloat3(&m_atVel);
    XMStoreFloat3(&m_eyeCur, SmoothDampV(XMLoadFloat3(&m_eyeCur), vEye, eVel, m_fSmoothTime, fTimeDelta));
    XMStoreFloat3(&m_atCur, SmoothDampV(XMLoadFloat3(&m_atCur), vAt, aVel, m_fSmoothTime, fTimeDelta));
    XMStoreFloat3(&m_eyeVel, eVel); XMStoreFloat3(&m_atVel, aVel);

    m_fFovy = XMConvertToRadians(m_fFovDeg);
    Recalculate_ProjMatrix();

    _vector vE = XMLoadFloat3(&m_eyeCur);
    _vector vLook = XMVectorSubtract(XMLoadFloat3(&m_atCur), vE);
    if (XMVectorGetX(XMVector3LengthSq(vLook)) < 1e-6f) vLook = XMVectorSet(0, 0, 1, 0);
    vLook = XMVector3Normalize(vLook);

    // 월드업 기준으로 basis 재구성 -> 롤(뱅킹) 제거, 화면 수평 유지
    _vector vR = XMVector3Cross(vUp, vLook);
    if (XMVectorGetX(XMVector3LengthSq(vR)) < 1e-6f) vR = XMVectorSet(1, 0, 0, 0);   // 수직 급상승/급강하 특이점 방어
    vR = XMVector3Normalize(vR);
    _vector vU = XMVector3Normalize(XMVector3Cross(vLook, vR));

    _matrix CamWorld;
    CamWorld.r[0] = XMVectorSetW(vR, 0.f);
    CamWorld.r[1] = XMVectorSetW(vU, 0.f);
    CamWorld.r[2] = XMVectorSetW(vLook, 0.f);
    CamWorld.r[3] = XMVectorSetW(vE, 1.f);

    Apply_Shake(CamWorld);

    m_pTransformCom->Set_State(STATE::RIGHT, CamWorld.r[0]);
    m_pTransformCom->Set_State(STATE::UP, CamWorld.r[1]);
    m_pTransformCom->Set_State(STATE::LOOK, CamWorld.r[2]);
    m_pTransformCom->Set_State(STATE::POSITION, CamWorld.r[3]);

    __super::Priority_Update(fTimeDelta);
}

HRESULT CCamera_Coaster::Ready_Events()
{
    return Ready_ShakeEvents();   // Camera_Shake / Camera_Rumble 구독
}

CCamera_Coaster* CCamera_Coaster::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCamera_Coaster* p = new CCamera_Coaster(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Create : CCamera_Coaster"); Safe_Release(p); }
    return p;
}
CGameObject* CCamera_Coaster::Clone(void* pArg)
{
    CCamera_Coaster* p = new CCamera_Coaster(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Clone : CCamera_Coaster"); Safe_Release(p); }
    return p;
}
void CCamera_Coaster::Free() { __super::Free(); }