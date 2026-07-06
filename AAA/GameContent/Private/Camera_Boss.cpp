#include "Camera_Boss.h"
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

CCamera_Boss::CCamera_Boss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) 
    : CCamera_Shakeable(pDevice, pContext) {}
CCamera_Boss::CCamera_Boss(const CCamera_Boss& Prototype) 
    : CCamera_Shakeable(Prototype) {}

HRESULT CCamera_Boss::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    m_bActive = false;                    
    return S_OK;
}

void CCamera_Boss::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) { __super::Priority_Update(fTimeDelta); return; }

    Tick_Shake(fTimeDelta);

    // 이벤트 쿼리로 타깃 확보(없을 때만)
    if (!m_pPlayer) { PLAYER_QUERY q{}; m_pGameInstance_Proxy->Publish(EVT_QUERY_PLAYER, &q); m_pPlayer = q.pPlayer; }
    if (!m_pBoss) { BOSS_QUERY   q{}; m_pGameInstance_Proxy->Publish(EVT_QUERY_BOSS, &q); m_pBoss = q.pBoss; }

    _vector P = m_pPlayer ? m_pPlayer->Get_Transform()->Get_State(STATE::POSITION) : XMVectorSet(0, 0, 0, 1);
    _vector B = m_pBoss ? m_pBoss->Get_Transform()->Get_State(STATE::POSITION) : XMVectorAdd(P,
        XMVectorSet(0, 0, 5, 0));

    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    // 커비->보스 수평 = 전방
    _vector vFwd = XMVectorSetY(XMVectorSubtract(B, P), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vFwd)) < 1e-6f) vFwd = XMVectorSet(0, 0, 1, 0);
    vFwd = XMVector3Normalize(vFwd);
    _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vFwd));   // 카메라 우측

    // eye: 커비 뒤 + 위 + 어깨(왼쪽)
    _vector vEye = XMVectorAdd(P, XMVectorScale(vUp, m_fHeight));
    vEye = XMVectorSubtract(vEye, XMVectorScale(vFwd, m_fBackDist));
    vEye = XMVectorAdd(vEye, XMVectorScale(vRight, m_fShoulderOffset));

    // at: 보스 쪽으로 프레이밍(어깨 너머 보스)
    _vector vAt = XMVectorLerp(P, B, m_fAimBias);
    vAt = XMVectorAdd(vAt, XMVectorScale(vUp, m_fAimHeight));

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
    _vector vR = XMVector3Cross(vUp, vLook);
    if (XMVectorGetX(XMVector3LengthSq(vR)) < 1e-6f) vR = XMVectorSet(1, 0, 0, 0);
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

HRESULT CCamera_Boss::Ready_Events()
{
    return Ready_ShakeEvents();
}

CCamera_Boss* CCamera_Boss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCamera_Boss* p = new CCamera_Boss(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Create : CCamera_Boss"); Safe_Release(p); }
    return p;
}
CGameObject* CCamera_Boss::Clone(void* pArg)
{
    CCamera_Boss* p = new CCamera_Boss(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Clone : CCamera_Boss"); Safe_Release(p); }
    return p;
}
void CCamera_Boss::Free() { __super::Free(); }