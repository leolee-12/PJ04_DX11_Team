#include "Kirby_Stuffed.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Stuffed::CKirby_Stuffed()
{
}

HRESULT CKirby_Stuffed::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_eCurStuffedState = STUFFED_STATE::STUFFED_END;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Stuffed::Get_StateType()
{
    return KIRBY_STATE_TYPE::FULL;
}

void CKirby_Stuffed::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Animation
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::STUFFED_START));

    // Body
    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Body(KIRBY_BODY_STATE::STUFFED);

    // State
    m_eCurStuffedState = STUFFED_START;
}

void CKirby_Stuffed::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator(); 

    Update_State(pKirby);
}

void CKirby_Stuffed::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    m_eCurStuffedState = STUFFED_STATE::STUFFED_END;
}

_bool CKirby_Stuffed::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
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

void CKirby_Stuffed::Update_State(CKirby* pKirby)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    switch (m_eCurStuffedState)
    {
    case STUFFED_STATE::STUFFED_START:
    {
        if (pAnimator->Is_Finished())
        {
            m_eCurStuffedState = STUFFED_STATE::STUFFED_WAIT;
            pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::STUFFED_WAIT));
        }

            break;
    }

    case STUFFED_STATE::STUFFED_WAIT:
        break;
    }
}

void CKirby_Stuffed::Enter_Animation(CKirby* pKirby, CAnimator* pAnimator)
{
}

CKirby_Stuffed* CKirby_Stuffed::Create()
{
    CKirby_Stuffed* pInstance = new CKirby_Stuffed();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Stuffed");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Stuffed::Free()
{
    __super::Free();
}