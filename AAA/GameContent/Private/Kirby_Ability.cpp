#include "Kirby_Ability.h"

#include "Kirby.h"

CKirby_Ability::CKirby_Ability()
{
}

HRESULT CKirby_Ability::Initialize()
{
    m_tAniInfos.resize(ETOUI(ABILITY_ANI::END));

    _uint iWait = ETOUI(ABILITY_ANI::WAIT);
    m_tAniInfos[iWait].strAniName = "Wait";
    m_tAniInfos[iWait].fBlend = 0.1f;
    m_tAniInfos[iWait].fSpeed = 1.8f;

    _uint iRun = ETOUI(ABILITY_ANI::RUN);
    m_tAniInfos[iRun].strAniName = "Run";
    m_tAniInfos[iRun].fBlend = 0.1f;
    m_tAniInfos[iRun].fSpeed = 1.8f;

    _uint iFall = ETOUI(ABILITY_ANI::FALL);
    m_tAniInfos[iFall].strAniName = "Fall";
    m_tAniInfos[iFall].bLoop = false;
    m_tAniInfos[iFall].fBlend = 0.1f;
    m_tAniInfos[iFall].fSpeed = 2.f;

    _uint iJumpL = ETOUI(ABILITY_ANI::JUMP_L);
    m_tAniInfos[iJumpL].strAniName = "JumpL";
    m_tAniInfos[iJumpL].bLoop = false;
    m_tAniInfos[iJumpL].fBlend = 0.1f;
    m_tAniInfos[iJumpL].fSpeed = 5.f;

    _uint iJumpR = ETOUI(ABILITY_ANI::JUMP_R);
    m_tAniInfos[iJumpR].strAniName = "JumpR";
    m_tAniInfos[iJumpR].bLoop = false;
    m_tAniInfos[iJumpR].fBlend = 0.1f;
    m_tAniInfos[iJumpR].fSpeed = 5.f;

    _uint iJumpEndL = ETOUI(ABILITY_ANI::JUMP_END_L);
    m_tAniInfos[iJumpEndL].strAniName = "JumpEndL";
    m_tAniInfos[iJumpEndL].bLoop = false;
    m_tAniInfos[iJumpEndL].fBlend = 0.1f;
    m_tAniInfos[iJumpEndL].fSpeed = 2.f;

    _uint iJumpEndR = ETOUI(ABILITY_ANI::JUMP_END_R);
    m_tAniInfos[iJumpEndR].strAniName = "JumpEndR";
    m_tAniInfos[iJumpEndR].bLoop = false;
    m_tAniInfos[iJumpEndR].fBlend = 0.1f;
    m_tAniInfos[iJumpEndR].fSpeed = 2.f;

    _uint iLanding = ETOUI(ABILITY_ANI::LANDING);
    m_tAniInfos[iLanding].strAniName = "Landing";
    m_tAniInfos[iLanding].bLoop = false;
    m_tAniInfos[iLanding].fBlend = 0.05f;
    m_tAniInfos[iLanding].fSpeed = 1.f;

    return S_OK;
}

_bool CKirby_Ability::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
        case KIRBY_ATTACK_LOCATION::GROUND:
        case KIRBY_ATTACK_LOCATION::AIR:
            return true;
    }

    return false;
}

const CAnimator::ANI_PLAY_INFO* CKirby_Ability::Get_AniInfo(ABILITY_ANI eAbilityAni)
{
    return &m_tAniInfos[ETOUI(eAbilityAni)];
}

void CKirby_Ability::Free()
{
    __super::Free();
}
