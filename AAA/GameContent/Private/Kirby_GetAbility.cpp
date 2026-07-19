#include "Kirby_GetAbility.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_GetAbility::CKirby_GetAbility()
{
}

HRESULT CKirby_GetAbility::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_GetAbility::Get_StateType()
{
    return KIRBY_STATE_TYPE::GET_ABILITY;
}

void CKirby_GetAbility::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    // Ani
    CKirby_Ability* pPreAbility = pKirby->Get_KirbyAbility();
    pKirby->Set_AbilityPartsActive(pPreAbility->Get_AbilityType(), false);
    pKirby->Apply_ChangeKirbyAbility();

    CKirby_Ability* pNewAbility = pKirby->Get_KirbyAbility();
    pNewAbility->Clear_Overlay(pKirby, 1, 0.1f);
    if (iFlag == GETABILITY_STATE_FLAG::ESSENCE)
        pNewAbility->Play_AbilityAni(pKirby, ABILITY_ANI::COPY);
    else
        pNewAbility->Play_AbilityAni(pKirby, ABILITY_ANI::GET_ABILITY);

    m_iGetAbilityStateFlag = iFlag;
    m_bPartsOn = false;

    KIRBY_ABILITY_CHANGED tDesc{};
    tDesc.bBegin = true;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &tDesc);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_Velocity(XMVectorSet(0.f, 0.f, 0.f, 0.f));
    pMovement->Clear_Forces();
    pMovement->Set_GravityScale(0.f);

    m_pGameInstance_Proxy->Set_TimeScale(0.f);

    KIRBY_NAME_UPDATED tNameDesc{};
    tNameDesc.strAtkModeName = pKirby->Get_ActiveAttackMode()->Get_AttackModeName();
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tNameDesc);
}

void CKirby_GetAbility::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CMovement_Child* pMovement = pKirby->Get_Movement();

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const _float fRatio = pAnimator->Get_Progress();

    _vector vCamPos = XMLoadFloat4(m_pGameInstance_Proxy->Get_CamPosition());
    _vector vPlayerPos = pKirby->Get_Transform()->Get_State(STATE::POSITION);
    _vector vDir = XMVectorSetY(vCamPos - vPlayerPos, 0.f);
    vDir = XMVector3Normalize(vDir);

    pMovement->Rotate_To_Direction(vDir, fTimeDelta);

    Parts_On(pKirby, fRatio);

    if(pAnimator->Is_Finished() == true)
    {
        if(pKirby->Get_ActiveAttackMode()->Should_ForceEnterAttackState())
            pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
        else
            Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
    }
}

void CKirby_GetAbility::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_KirbyMouth(KIRBY_MOUTH_STATE::IDLE);

    KIRBY_ABILITY_CHANGED Desc{};
    Desc.bBegin = false;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &Desc);
    
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GravityScale(1.f);

    m_pGameInstance_Proxy->Set_TimeScale(1.f);
}

_bool CKirby_GetAbility::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_GetAbility::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

void CKirby_GetAbility::Parts_On(CKirby* pKirby, _float fRatio)
{
    if (m_iGetAbilityStateFlag == GETABILITY_STATE_FLAG::ESSENCE)
    {
        if (m_bPartsOn == false && fRatio >= 0.0f)
        {
            pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), true);

            m_bPartsOn = true;
        }
    }
    else
    {
        if (m_bPartsOn == false && fRatio >= 0.3f)
        {
            pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), true);

            m_bPartsOn = true;
        }
    }

}

CKirby_GetAbility* CKirby_GetAbility::Create()
{
    CKirby_GetAbility* pInstance = new CKirby_GetAbility();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_GetAbility");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_GetAbility::Free()
{
    __super::Free();
}