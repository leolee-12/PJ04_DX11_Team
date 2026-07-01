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

    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_GetAbility::Get_StateType()
{
    return KIRBY_STATE_TYPE::GET_ABILITY;
}

void CKirby_GetAbility::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::GET_ABILITY);

    m_bPartsOn = false;
    m_bCloseEye = false;
    m_bOpenMouse = false;
}

void CKirby_GetAbility::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CMovement_Child* pMovement = pKirby->Get_Movement();

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const _float fRatio = pAnimator->Get_Progress();

    _vector vCamLook = XMVectorSetY(XMLoadFloat4(m_pGameInstance_Proxy->Get_CamLook()), 0.f);
    pMovement->Rotate_To_Direction(-vCamLook, fTimeDelta);

    if(pAnimator->Is_Finished() == true)
    {
        Transition_Fall_OR_Wait_OR_Run(pKirby);
    }

    Parts_On(pKirby, fRatio);
    Close_Eye(pBody, fRatio);
    Open_Mouse(pBody, fRatio);
}

void CKirby_GetAbility::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_KirbyMouth(KIRBY_MOUTH_STATE::IDLE);
}

_bool CKirby_GetAbility::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    //switch (eCommandType)
    //{
    //    default:        
    //        break;
    //}

    return false;
}

void CKirby_GetAbility::Parts_On(CKirby* pKirby, _float fRatio)
{
    if (m_bPartsOn == false && fRatio >= 0.3f)
    {
        pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), true);

        m_bPartsOn = true;
    }
}

void CKirby_GetAbility::Close_Eye(CKirby_Body* pBody, _float fRatio)
{
    if (m_bCloseEye == false && fRatio >= 0.3f && fRatio < 0.55f)
    {
        pBody->Set_KirbyEye(KIRBY_EYE_STATE::CLOSE);

        m_bCloseEye = true;
    }
    else if (m_bCloseEye == true && fRatio >= 0.55f)
    {
        pBody->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);

        m_bCloseEye = false;
    }
}

void CKirby_GetAbility::Open_Mouse(CKirby_Body* pBody, _float fRatio)
{
    if (m_bOpenMouse == false && fRatio >= 0.55f)
    {
        pBody->Set_KirbyMouth(KIRBY_MOUTH_STATE::SMILE_OPEN);

        m_bOpenMouse = true;
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
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}
