#include "BTNode.h"
#include "Animator.h"

CBTAction* CBTAction::Create(TickFn fn, ResetFn rfn)
{
    CBTAction* pInstance = new CBTAction();
    pInstance->m_Fn = move(fn);
    pInstance->m_ResetFn = move(rfn);
    return pInstance;
}

void CBTAction::Free()
{
    __super::Free();
}

CBTCondition* CBTCondition::Create(CondFn fn)
{
    CBTCondition* pInstance = new CBTCondition();
    pInstance->m_Fn = move(fn);
    return pInstance;
}

void CBTCondition::Free()
{
    __super::Free();
}

BT_STATUS CBTPlayClip::Tick(CBlackboard* pBB, _float fDt)
{
    CAnimator* pAnim = m_Getter ? m_Getter() : nullptr;
    if (nullptr == pAnim) return BT_STATUS::FAILURE;

    if (!m_bStarted)
    {
        pAnim->Play(m_Desc.clip, m_Desc.bLoop, true, m_Desc.fBlend, m_Desc.fSpeed);
        m_bStarted = true;
    }

    if (m_Desc.bLoop)
    {
        m_fT += fDt;
        if (m_fT >= m_Desc.fHold) { Reset(); return BT_STATUS::SUCCESS; }
        return BT_STATUS::RUNNING;
    }

    if (pAnim->Is_Finished()) { Reset(); return BT_STATUS::SUCCESS; }
    return BT_STATUS::RUNNING;
}

CBTPlayClip* CBTPlayClip::Create(AnimGetter getter, const DESC& desc)
{
    CBTPlayClip* pInstance = new CBTPlayClip();
    pInstance->m_Getter = move(getter);
    pInstance->m_Desc = desc;
    return pInstance;
}

void CBTPlayClip::Free()
{
    __super::Free();
}
