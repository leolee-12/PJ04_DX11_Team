#include "Gigatzo_Brain.h"
#include "Gigatzo.h"

CGigatzo_Brain::CGigatzo_Brain()
{
}

HRESULT CGigatzo_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CGigatzo_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard,  _float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    if (nullptr == BlackBoard.pTarget)
    {
        m_bArmed = false;
        m_bPassLatched = false;
        m_bHasPrevTarget = false;
        m_bHasPrevSpeed = false;
        return;
    }

    const _vector vTargetPos = XMLoadFloat3(&BlackBoard.vTargetPos);

    _vector vTargetVel = XMVectorZero();
    _float  fTargetSpeed = 0.f;
    _bool   bVelValid = false;

    if (m_bHasPrevTarget && fTimeDelta > 0.f)
    {
        const _vector vDelta =
            vTargetPos - XMLoadFloat3(&m_vPrevTargetPos);

        const _float fStep =
            XMVectorGetX(XMVector3Length(vDelta));

        if (fStep <= s_fTeleportStep)
        {
            vTargetVel = vDelta / fTimeDelta;
            fTargetSpeed =
                XMVectorGetX(XMVector3Length(vTargetVel));
            bVelValid = true;
        }
    }

    XMStoreFloat3(&m_vPrevTargetPos, vTargetPos);
    m_bHasPrevTarget = true;

    // target acceleration estimate (slope accel / arrival decel both covered)
    _float fRawAccel = 0.f;
    if (m_bHasPrevSpeed && bVelValid && fTimeDelta > 0.f)
        fRawAccel = (fTargetSpeed - m_fPrevTargetSpeed) / fTimeDelta;

    Helper::FloatClamp(fRawAccel, -s_fMaxTargetAccel, s_fMaxTargetAccel);

    const _float fAccelBlend = min(1.f, fTimeDelta * s_fAccelSmoothRate);
    m_fTargetAccel += (fRawAccel - m_fTargetAccel) * fAccelBlend;

    if (bVelValid)
    {
        m_fPrevTargetSpeed = fTargetSpeed;
        m_bHasPrevSpeed = true;
        m_fObservedMaxSpeed = max(m_fObservedMaxSpeed, fTargetSpeed);
    }

    CGigatzo* pGig = static_cast<CGigatzo*>(m_pOwner);

    if (pGig->Is_RenderCulled() || !pGig->Is_InCameraFront())
    {
        m_bArmed = false;
        return;
    }

    if (!bVelValid)
        return;

    if (fTargetSpeed <= s_fMinLeadSpeed || pGig->Is_LongRange())
    {
        m_bPassLatched = false;

        if (!m_bArmed)
        {
            m_fFireTimer = 0.f;
            m_bArmed = true;
        }

        m_fFireTimer -= fTimeDelta;
        if (m_fFireTimer > 0.f)
            return;

        m_fFireTimer += s_fFireInterval;

        if (!Can_Decide(BlackBoard))
            return;
        if (MONSTER_STATE_TYPE::IDLE != m_pOwner->Get_StateType())
            return;

        m_pOwner->Change_State(MONSTER_STATE_TYPE::ATTACK);
        return;
    }

    m_bArmed = false;

    // muzzle ray: exact for Vertical / Tilt / Horizontal pitch
    _vector vMuzzlePos{}, vMuzzleDir{};
    if (!pGig->Get_MuzzleRay(&vMuzzlePos, &vMuzzleDir))
        return;

    const _vector vForward = XMVector3Normalize(vTargetVel);

    // closest approach between target path L1(t) = vTargetPos + t*vForward
    // and muzzle ray      L2(s) = vMuzzlePos + s*vMuzzleDir
    // t = distance target must travel, s = bullet flight distance
    const _vector vR = vTargetPos - vMuzzlePos;

    const _float fB = XMVectorGetX(XMVector3Dot(vForward, vMuzzleDir));
    const _float fD = XMVectorGetX(XMVector3Dot(vForward, vR));
    const _float fE = XMVectorGetX(XMVector3Dot(vMuzzleDir, vR));

    const _float fDenom = 1.f - fB * fB;      // always >= 0 (unit vectors)
    if (fDenom < Helper::fEpsilon)
        return;                               // barrel parallel to path

    const _float fForward    = (fB * fE - fD) / fDenom;
    const _float fFlightDist = (fE - fB * fD) / fDenom;

    if (fForward < -s_fRearmDistance)
        m_bPassLatched = false;

    if (m_bPassLatched || fForward <= 0.f)
        return;

    if (fFlightDist <= 0.f)
        return;                               // intercept is behind the muzzle

    const _float fBulletSpeed = pGig->Get_BulletSpeed();
    if (fBulletSpeed <= Helper::fEpsilon)
        return;

    const _float fT = s_fAnimDelay + fFlightDist / fBulletSpeed;

    // no constant-velocity assumption. upper bound is the observed max,
    // never an external constant owned by another system.
    _float fEndSpeed = fTargetSpeed + m_fTargetAccel * fT;

    if (m_fObservedMaxSpeed > 0.f)
        fEndSpeed = min(fEndSpeed, m_fObservedMaxSpeed);

    fEndSpeed = max(fEndSpeed, 0.f);

    const _float fLead = 0.5f * (fTargetSpeed + fEndSpeed) * fT + s_fLeadBias;

    if (fLead > pGig->Get_FireDistance())
        return;                               // out of range: skip, do not clamp

    if (fForward > fLead)
        return;

    if (!Can_Decide(BlackBoard))
        return;
    if (MONSTER_STATE_TYPE::IDLE != m_pOwner->Get_StateType())
        return;

    m_bPassLatched = true;
    m_pOwner->Change_State(MONSTER_STATE_TYPE::ATTACK);
}

CGigatzo_Brain* CGigatzo_Brain::Create(CMonster* pOwner)
{
	CGigatzo_Brain* pInstance = new CGigatzo_Brain();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CGigatzo_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigatzo_Brain::Free()
{
	__super::Free();
}
