#include "Kirby_Ability.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Movement_Child.h"

CKirby_Ability::CKirby_Ability()
{
}

HRESULT CKirby_Ability::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_AniInfos.resize(ETOUI(ABILITY_ANI::END));

    Set_FullBodyAni(ABILITY_ANI::WAIT, "Wait", true, false, 0.1f, 1.8f);
    Set_FullBodyAni(ABILITY_ANI::RUN, "Run", true, false, 0.1f, 3.5f);

    Set_FullBodyAni(ABILITY_ANI::FALL, "Fall", false, false, 0.1f, 2.f);

    Set_FullBodyAni(ABILITY_ANI::JUMP_L, "JumpL", false, false, 0.1f, 5.f);
    Set_FullBodyAni(ABILITY_ANI::JUMP_R, "JumpR", false, false, 0.1f, 5.f);

    Set_FullBodyAni(ABILITY_ANI::JUMP_END_L, "JumpEndL", false, false, 0.1f, 2.f);
    Set_FullBodyAni(ABILITY_ANI::JUMP_END_R, "JumpEndR", false, false, 0.1f, 2.f);

    Set_FullBodyAni(ABILITY_ANI::LANDING, "Landing", false, false, 0.05f, 1.f);

    Set_FullBodyAni(ABILITY_ANI::GET_ABILITY,   "GetAbilityFirst", false, false, 0.1f, 2.5f);
    Set_FullBodyAni(ABILITY_ANI::COPY,          "Copy", false, false, 0.1f, 2.5f);
    Set_FullBodyAni(ABILITY_ANI::ABILITY_DUMP,  "AbilityDump", false, false, 0.1f, 2.5f);

    Set_FullBodyAni(ABILITY_ANI::FLIGHT_START, "FlightStart", false, false, 0.1f, 2.25f);
    Set_FullBodyAni(ABILITY_ANI::FLIGHT, "Flight", false, true, 0.1f, 2.f);
    Set_FullBodyAni(ABILITY_ANI::FLIGHT_FALL, "FlightFall", true, false, 0.1f, 2.f);
    Set_FullBodyAni(ABILITY_ANI::FLIGHT_LANDING, "FlightLanding", false, false, 0.1f, 2.5f);
    Set_FullBodyAni(ABILITY_ANI::AIR_BALL, "AirBall", false, false, 0.0f, 5.f);

    Set_FullBodyAni(ABILITY_ANI::DAMAGED, "Damage", false, false, 0.1f, 1.5f);

    Set_FullBodyAni(ABILITY_ANI::GUARD, "Guard", true, true, 0.1f, 1.8f);

    Set_FullBodyAni(ABILITY_ANI::DODGE_START, "DodgeStart", false, false, 0.1f, 2.f);

    Set_FullBodyAni(ABILITY_ANI::SLIDE_START, "SlideStart", false, false, 0.1f, 1.5f);
    Set_FullBodyAni(ABILITY_ANI::SLIDE, "Slide", false, false, 0.1f, 1.5f);

    return S_OK;
}

_bool CKirby_Ability::Handle_BodyAnimEvent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::Hitbox)
        return false;

    if (ePhase == ANIM_EVENT_PHASE::BEGIN)
    {
        switch (static_cast<COMMON_HIT_PARAM>(e.iIntParam))
        {
            case COMMON_HIT_PARAM::SLIDE_H:
            {
                pKirby->Get_Collider(CKirby::KIRBY_COLLIDER::SLIDE_COLLIDER)->Set_Enabled(true);
                return true;
            }
        }
    }

    if (ePhase == ANIM_EVENT_PHASE::END)
    {
        switch (static_cast<COMMON_HIT_PARAM>(e.iIntParam))
        {
            case COMMON_HIT_PARAM::SLIDE_H:
            {
                pKirby->Get_Collider(CKirby::KIRBY_COLLIDER::SLIDE_COLLIDER)->Set_Enabled(false);
                return true;
            }
        }
    }

    return false;
}

const CAnimator::ANI_PLAY_INFO* CKirby_Ability::Get_AniInfo(ABILITY_ANI eAbilityAni)
{
    return &m_AniInfos[ETOUI(eAbilityAni)].tBaseAniInfo;
}

void CKirby_Ability::Play_AbilityAni(CKirby* pKirby, ABILITY_ANI eAbilityAni)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const KIRBY_TYPE_ANI_DESC& tDesc = m_AniInfos[ETOUI(eAbilityAni)];

    pAnimator->Play(&tDesc.tBaseAniInfo);

    if (tDesc.ePlayType == ANI_PLAY_TYPE::OVERLAY)
        pAnimator->Apply_Overlay(tDesc.tLayerAniInfo);
}

void CKirby_Ability::Set_FullBodyAni(ABILITY_ANI eAni, const _string& strAniName, _bool bLoop, _bool bRestart, _float fBlend, _float fSpeed)
{
    KIRBY_TYPE_ANI_DESC& desc = m_AniInfos[ETOUI(eAni)];
    desc.ePlayType = ANI_PLAY_TYPE::FULL_BODY;

    desc.tBaseAniInfo.strAniName = strAniName;
    desc.tBaseAniInfo.bLoop = bLoop;
    desc.tBaseAniInfo.bRestart = bRestart;
    desc.tBaseAniInfo.fBlend = fBlend;
    desc.tBaseAniInfo.fSpeed = fSpeed;
}

void CKirby_Ability::Set_OverlayAni(ABILITY_ANI eAni, const _string& strBaseAniName, const _string& strOverlayAniName, const _string& strRootBone,
    _bool bBaseLoop, _bool bBaseRestart, _float fBaseSpeed, _float fBaseBlend,
    _bool bOverlayLoop, _bool bOverlayRestart, _float fOverlaySpeed, _float fTargetWeight, _float fWeightBlendTime,
    _float fOverlayClipBlend)
{
    KIRBY_TYPE_ANI_DESC& desc = m_AniInfos[ETOUI(eAni)];

    desc.ePlayType = ANI_PLAY_TYPE::OVERLAY;

    desc.tBaseAniInfo.strAniName = strBaseAniName;
    desc.tBaseAniInfo.bLoop = bBaseLoop;
    desc.tBaseAniInfo.bRestart = bBaseRestart;
    desc.tBaseAniInfo.fBlend = fBaseBlend;
    desc.tBaseAniInfo.fSpeed = fBaseSpeed;

    CAnimator::LAYER_PLAY_INFO& tLayer = desc.tLayerAniInfo;

    tLayer.iSlot = 1;
    tLayer.tAnim.strAniName = strOverlayAniName;
    tLayer.tAnim.bLoop = bOverlayLoop;
    tLayer.tAnim.bRestart = bOverlayRestart;
    tLayer.tAnim.fBlend = fOverlayClipBlend;
    tLayer.tAnim.fSpeed = fOverlaySpeed;

    tLayer.Roots = { strRootBone };
    tLayer.fTargetWeight = fTargetWeight;
    tLayer.fWeightBlend = fWeightBlendTime;
}

void CKirby_Ability::Free()
{
    __super::Free();
}
