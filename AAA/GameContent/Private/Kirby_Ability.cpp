#include "Kirby_Ability.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Ability::CKirby_Ability()
{
}

HRESULT CKirby_Ability::Initialize()
{
    m_tAniInfos.resize(ETOUI(ABILITY_ANI::END));

    Set_FullBodyAni(ABILITY_ANI::WAIT, "Wait", true, false, 0.1f, 1.8f);
    Set_FullBodyAni(ABILITY_ANI::RUN, "Run", true, false, 0.1f, 2.5f);

    Set_FullBodyAni(ABILITY_ANI::FALL, "Fall", false, false, 0.1f, 2.f);

    Set_FullBodyAni(ABILITY_ANI::JUMP_L, "JumpL", false, false, 0.1f, 5.f);
    Set_FullBodyAni(ABILITY_ANI::JUMP_R, "JumpR", false, false, 0.1f, 5.f);

    Set_FullBodyAni(ABILITY_ANI::JUMP_END_L, "JumpEndL", false, false, 0.1f, 2.f);
    Set_FullBodyAni(ABILITY_ANI::JUMP_END_R, "JumpEndR", false, false, 0.1f, 2.f);

    Set_FullBodyAni(ABILITY_ANI::LANDING, "Landing", false, false, 0.05f, 1.f);

    Set_FullBodyAni(ABILITY_ANI::GET_ABILITY, "GetAbilityFirst", false, false, 0.1f, 2.5f);
    Set_FullBodyAni(ABILITY_ANI::ABILITY_DUMP, "AbilityDump", false, false, 0.1f, 2.5f);

    Set_FullBodyAni(ABILITY_ANI::FLIGHT_START, "FlightStart", false, false, 0.1f, 2.25f);
    Set_FullBodyAni(ABILITY_ANI::FLIGHT, "Flight", false, true, 0.1f, 2.f);
    Set_FullBodyAni(ABILITY_ANI::FLIGHT_FALL, "FlightFall", true, false, 0.1f, 2.f);
    Set_FullBodyAni(ABILITY_ANI::FLIGHT_LANDING, "FlightLanding", false, false, 0.1f, 2.5f);
    Set_FullBodyAni(ABILITY_ANI::AIR_BALL, "AirBall", false, false, 0.0f, 5.f);

    Set_FullBodyAni(ABILITY_ANI::DAMAGED, "Damage", false, false, 0.1f, 1.5f);

    Set_FullBodyAni(ABILITY_ANI::GUARD, "Guard", true, true, 0.1f, 1.8f);

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
    return &m_tAniInfos[ETOUI(eAbilityAni)].tBaseAniInfo;
}

void CKirby_Ability::Play_AbilityAni(CKirby* pKirby, ABILITY_ANI eAbilityAni)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const ABILITY_ANI_DESC& tDesc = m_tAniInfos[ETOUI(eAbilityAni)];

    pAnimator->Play(&tDesc.tBaseAniInfo);

    if (tDesc.ePlayType == ABILITY_ANI_PLAY_TYPE::OVERLAY)
    {
        pAnimator->Set_Mask(
            tDesc.tOverlayAniInfo.strAniName.c_str(),
            tDesc.strOverlayRootBone.c_str(),
            tDesc.tOverlayAniInfo.bLoop,
            tDesc.fOverlayWeight,
            tDesc.fOverlayBlend
        );
    }
}

void CKirby_Ability::Clear_Overlay(CKirby* pKirby, _uint iSlot, _float fOverlayBlendTime)
{
    pKirby->Get_Body()->Get_Animator()->Clear_Overlay(iSlot, fOverlayBlendTime);
}

void CKirby_Ability::Set_FullBodyAni(ABILITY_ANI eAni, const _string& strAniName, _bool bLoop, _bool bRestart, _float fBlend, _float fSpeed)
{
    ABILITY_ANI_DESC& desc = m_tAniInfos[ETOUI(eAni)];

    desc.ePlayType = ABILITY_ANI_PLAY_TYPE::FULL_BODY;

    desc.tBaseAniInfo.strAniName = strAniName;
    desc.tBaseAniInfo.bLoop = bLoop;
    desc.tBaseAniInfo.bRestart = bRestart;
    desc.tBaseAniInfo.fBlend = fBlend;
    desc.tBaseAniInfo.fSpeed = fSpeed;
}

void CKirby_Ability::Set_OverlayAni(ABILITY_ANI eAni, const _string& strBaseAniName, const _string& strOverlayAniName, const _string& strRootBone,
    _bool bBaseLoop, _bool bBaseRestart, _float fBaseSpeed, _float fBaseBlend, _bool bBaseClearMask,
    _bool bOverlayLoop, _bool bOverlayRestart, _float fOverlaySpeed, _float fOverlaysBlend)
{
    ABILITY_ANI_DESC& desc = m_tAniInfos[ETOUI(eAni)];

    desc.ePlayType = ABILITY_ANI_PLAY_TYPE::OVERLAY;

    desc.tBaseAniInfo.strAniName = strBaseAniName;
    desc.tBaseAniInfo.bLoop = bBaseLoop;
    desc.tBaseAniInfo.bRestart = bBaseRestart;
    desc.tBaseAniInfo.fBlend = fBaseBlend;
    desc.tBaseAniInfo.fSpeed = fBaseSpeed;
    desc.tBaseAniInfo.bClearMask = bBaseClearMask;

    desc.tOverlayAniInfo.strAniName = strOverlayAniName;
    desc.tOverlayAniInfo.bLoop = bOverlayLoop;
    desc.tOverlayAniInfo.bRestart = bOverlayRestart;
    desc.tOverlayAniInfo.fBlend = fOverlaysBlend;
    desc.tOverlayAniInfo.fSpeed = fOverlaySpeed;
    desc.tOverlayAniInfo.bClearMask = bBaseClearMask;

    desc.strOverlayRootBone = strRootBone;
    desc.fOverlayWeight = 1.f;
    desc.fOverlayBlend = fOverlaysBlend;
}

void CKirby_Ability::Free()
{
    __super::Free();
}
