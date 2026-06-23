#include "Kirby_Jump.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

CKirby_Jump::CKirby_Jump()
{
}

HRESULT CKirby_Jump::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_fMaxGroundIgnoreTime = 0.5f;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Jump::Get_StateType()
{
    return KIRBY_STATE_TYPE::JUMP;
}

void CKirby_Jump::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    // Movement Jump
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    pMovementCom->Try_Jump();

    // Ani
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();

    if (m_bLeft == true)
        pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::JUMP_L);
    else
        pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::JUMP_R);

    // Ground Ignore
    m_fAccGroundIgnoreTime = m_fMaxGroundIgnoreTime;

    // Jump State
    m_eJumpType = JUMP_STATE::JUMP_STRAT;
}

void CKirby_Jump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    // 점프 직후 잠깐은 착지 판정 무시
    if (m_fAccGroundIgnoreTime > 0.f)
    {
        m_fAccGroundIgnoreTime -= fTimeDelta;
        if (m_fAccGroundIgnoreTime < 0.f)
            m_fAccGroundIgnoreTime = 0.f;

        return;
    }

    CMovement_Child* pMovementCom = pKirby->Get_Movement();

    if (pMovementCom->Is_Grounded() == true)
    {
        pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
        return;
    }
   
    switch (m_eJumpType)
    {
        case JUMP_STATE::JUMP_STRAT:
        {
            _float fYVelocity = pMovementCom->Get_VerticalVelocity();
            if (fYVelocity <= 0.f)
            {
                if (pKirby->Get_Body()->Get_BodyState() != KIRBY_BODY_STATE::STUFFED)
                {
                    if (rand() % 2 == 0)
                    {
                        m_eJumpType = JUMP_STATE::JUMP_END;

                        CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
                        if (m_bLeft == true)
                            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::JUMP_END_L);
                        else
                            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::JUMP_END_R);

                        return;
                    }
                }

                pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
            }

            break;
        }

        case JUMP_STATE::JUMP_END:
        {
            CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

            if (pAnimator->Is_Finished())
                pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
            break;
        }
    }
}

void CKirby_Jump::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    // 왼발 점프, 오른발 점프
    m_bLeft = !m_bLeft;
}

_bool CKirby_Jump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move Press
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }
        // Hovering
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            if(pKirby->Get_Body()->Get_BodyState() != KIRBY_BODY_STATE::STUFFED)
                pKirby->Change_State(KIRBY_STATE_TYPE::HOVERING);

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
            if (pAbility->Can_Attack(KIRBY_ATTACK_LOCATION::AIR))
            {
                if (pCommand->IsDown())
                    pAbility->Enter_Attack_KeyDown(pKirby);
                else if (pCommand->IsPress())
                    pAbility->Enter_Attack_KeyPress(pKirby);
                else if (pCommand->IsUp())
                    pAbility->Enter_Attack_KeyUp(pKirby);
            }

            return true;
        }
    }

    return false;
}

CKirby_Jump* CKirby_Jump::Create()
{
    CKirby_Jump* pInstance = new CKirby_Jump();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Jump");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Jump::Free()
{
    __super::Free();
}
