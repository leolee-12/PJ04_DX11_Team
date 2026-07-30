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

    CTransform* pGigTransform = pGig->Get_Transform();
    if (nullptr == pGigTransform)
        return;

    // 커비 속도 정규화
    const _vector vForward = XMVector3Normalize(vTargetVel);

    // 커비와 대포의 직선 거리
    const _vector vRelative = pGigTransform->Get_State(STATE::POSITION) - vTargetPos;

    // 커비가 대포 기준으로 어디에 있는지 판단 하는 기준 
    const _float fForward = XMVectorGetX(XMVector3Dot(vRelative, vForward));

    if (fForward < -s_fRearmDistance)
        m_bPassLatched = false;

    if (m_bPassLatched || fForward <= 0.f)
        return;

    const _vector vLateral = vRelative - vForward * fForward;
    const _float fLateral = XMVectorGetX(XMVector3Length(vLateral));

    const _float fBulletSpeed = pGig->Get_BulletSpeed();
    if (fBulletSpeed <= Helper::fEpsilon)
        return;

    _float fLead = fTargetSpeed *
        (s_fAnimDelay + fLateral / fBulletSpeed) + s_fLeadBias;

    fLead = min(fLead, pGig->Get_FireDistance());

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
