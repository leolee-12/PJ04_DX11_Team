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
    return KIRBY_STATE_TYPE::CUTSCENE_GRABBED;
}

void CKirby_Clear::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    pAnimator->Play("Cut1", false, false, 0.1f, 1.5f);

    m_eClearState = CLEAR_STATE::CLEAR_END;
    Change_ClearState(pKirby, CLEAR_STATE::CUT1);
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
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (eState)
    {
        case CLEAR_STATE::CUT1:
            pAnimator->Play("Cut1", false, false, 0.1f, 1.f);
            break;
        case CLEAR_STATE::DANCE:
            pAnimator->Play("ClearDanceLong", false, false, 0.1f, 1.f);
            break;
        case CLEAR_STATE::CLEAR_END:
            break;
    }
}

void CKirby_Clear::Update_ClearState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eClearState)
    {
        case CLEAR_STATE::CUT1:
        {
            if (pAnimator->Is_Finished())
                Change_ClearState(pKirby, CLEAR_STATE::DANCE);
            break;
        }
        case CLEAR_STATE::DANCE:
            break;
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
