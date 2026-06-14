#include "Kirby_Ability_Sword.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Ability_Sword::CKirby_Ability_Sword()
{
}

HRESULT CKirby_Ability_Sword::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_ABILITY_TYPE CKirby_Ability_Sword::Get_AbilityType()
{
    return KIRBY_ABILITY_TYPE::SWORD;
}

void CKirby_Ability_Sword::Enter_Ability(CKirby* pKirby)
{
}

void CKirby_Ability_Sword::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    m_bEndAttack = true;

    //CMovement_Child* pMovementCom = pKirby->Get_Movement();

    //CKirby_Body* pBody = pKirby->Get_Body();
    //CAnimator* pAnimator = pBody->Get_Animator();

    //_bool bIsAniFinish = pAnimator->Is_Finished();

    //switch (m_eCurSwordState)
    //{
    //    case SWORD_STATE::SLASH_1:
    //    {
    //        if (bIsAniFinish && m_bReserveNextAttack)
    //        {
    //            m_eCurSwordState = SWORD_STATE::SLASH_2;
    //        }
    //        else if(bIsAniFinish)
    //        {
    //            m_eCurSwordState = SWORD_STATE::SLASH_1_END;
    //        }
    //        break;
    //    }
    //    case SWORD_STATE::SLASH_2:
    //    {
    //        if (bIsAniFinish && m_bReserveNextAttack)
    //        {
    //            m_eCurSwordState = SWORD_STATE::SLASH_3;
    //        }
    //        break;
    //    }
    //    case SWORD_STATE::SLASH_3:
    //    {
    //        break;
    //    }
    //}


    //if (m_eCurSwordState != m_ePreSwordState)
    //{
    //    switch (m_eCurSwordState)
    //    {
    //        case SWORD_STATE::SLASH_1:
    //        {
    //            pAnimator->Play("SideSlash", false, false, 0.1f, 1.f);
    //            break;
    //        }
    //        
    //        case SWORD_STATE::SLASH_1_END:
    //        {
    //            pAnimator->Play("SideSlashEnd", false, false, 0.1f, 1.f);
    //            break;
    //        }

    //        case SWORD_STATE::SLASH_2:
    //        {
    //            pAnimator->Play("MultiswordAttack", false, false, 0.1f, 1.f);
    //            break;
    //        }

    //        case SWORD_STATE::SLASH_3:
    //        {
    //            pAnimator->Play("DecisiveSlash", false, false, 0.1f, 1.f);
    //            break;            
    //        }
    //    }

    //    m_ePreSwordState = m_eCurSwordState;
    //}

    //if(bIsAniFinish == true)
    //    m_bEndAttack = true;

    //debug
    char szLog[128] = {};
    sprintf_s(szLog, "Sword Attack\n");
    OutputDebugStringA(szLog);
}

void CKirby_Ability_Sword::Exit_Ability(CKirby* pKirby)
{
    m_eCurSwordState = SWORD_STATE::NONE;
    m_ePreSwordState = SWORD_STATE::NONE;
    m_bReserveNextAttack = false;
}

_bool CKirby_Ability_Sword::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eCurSwordState == SWORD_STATE::SLASH_3)
                return true;

                m_bReserveNextAttack = true;

            return true;
        }
    }

    return false;
}

void CKirby_Ability_Sword::Down_Attack(CKirby* pKirby)
{
    m_bEndAttack = false;

    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    m_eCurSwordState = SWORD_STATE::SLASH_1;
}

void CKirby_Ability_Sword::Up_Attack(CKirby* pKirby)
{
}

_bool CKirby_Ability_Sword::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
        case KIRBY_ATTACK_LOCATION::GROUND:     return true;
        case KIRBY_ATTACK_LOCATION::AIR:        return true;
    }

    return false;
}

CKirby_Ability_Sword* CKirby_Ability_Sword::Create()
{
    CKirby_Ability_Sword* pInstance = new CKirby_Ability_Sword();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Sword::Free()
{
    __super::Free();
}