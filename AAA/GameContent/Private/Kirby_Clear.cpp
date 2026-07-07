#include "Kirby_Clear.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_Clear::CKirby_Clear()
{
}

HRESULT CKirby_Clear::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Clear::Get_StateType()
{
    return KIRBY_STATE_TYPE::STAGE_CLEAR;
}

void CKirby_Clear::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);

    m_eClearState = CLEAR_STATE::CLEAR_END;
}

void CKirby_Clear::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_ClearState(pKirby, fTimeDelta);
}

void CKirby_Clear::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_Clear::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_Clear::Request_StageClear(CKirby* pKirby, const CUTSCENE_STAGECLEAR* pDesc)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (pDesc->eAnim)
    {
        case STAGECLEAR_ANIM::CUT1:
        {
            pKirby->Get_Transform()->Set_State(STATE::POSITION,
                XMVectorSetW(XMLoadFloat3(&pDesc->vPos), 1.f));

            pAnimator->Play("Cut1", false, false, pDesc->fBlendDuration, pDesc->fAnimSpeed);

            Change_ClearState(pKirby, CLEAR_STATE::CUT1);
            break;
        }
        case STAGECLEAR_ANIM::DANCE:
        {
            pAnimator->Play("ClearDanceLong", false, false, pDesc->fBlendDuration, pDesc->fAnimSpeed);
            Change_ClearState(pKirby, CLEAR_STATE::DANCE);
            break;
        }
    }
}

void CKirby_Clear::Change_ClearState(CKirby* pKirby, CLEAR_STATE eNext)
{
    if (m_eClearState == eNext)
        return;

    Exit_ClearState(pKirby, m_eClearState);

    m_eClearState = eNext;

    Enter_ClearState(pKirby, m_eClearState);
}

void CKirby_Clear::Enter_ClearState(CKirby* pKirby, CLEAR_STATE eState)
{
    switch (eState)
    {
        case CLEAR_STATE::CUT1:
            break;
        case CLEAR_STATE::DANCE:
            break;
        case CLEAR_STATE::CLEAR_END:
            break;
    }
}

void CKirby_Clear::Update_ClearState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eClearState)
    {
        case CLEAR_STATE::CUT1:
        {
            break;
        }
        case CLEAR_STATE::DANCE:
        {
            CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
            if (pAnimator->Is_Finished())
                Transition_Fall_OR_Wait_OR_Run(pKirby);
            break;
        }
        case CLEAR_STATE::CLEAR_END:
            break;
    }
}

void CKirby_Clear::Exit_ClearState(CKirby* pKirby, CLEAR_STATE eState)
{
    switch (eState)
    {
        case CLEAR_STATE::CUT1:
            break;
        case CLEAR_STATE::DANCE:
            break;
        case CLEAR_STATE::CLEAR_END:
            break;
    }
}

CKirby_Clear* CKirby_Clear::Create()
{
    CKirby_Clear* pInstance = new CKirby_Clear();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Clear");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Clear::Free()
{
    __super::Free();
}
