#include "Kirby_Jump.h"

#include "GameInstance.h"
#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

#include "Kirby_Fall.h"

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

void CKirby_Jump::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    // Movement Jump
    CMovement_Child* pMovementCom = pKirby->Get_Movement();
    if (iFlag == JUMP_STATE_FLAG::FORCE_JUMP)
        pMovementCom->Force_Jump();
    else
        pMovementCom->Try_Jump();

     // Ani
    if (pKirby->Has_Deform())
    {
        pKirby->Get_KirbyDeform()->Play_DeformAni(pKirby, DEFORM_ANI::JUMP_START);

        m_eJumpType = JUMP_STATE::JUMP_START;
    }
    else
    {
        CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
        ABILITY_ANI eJumpAni{};
        if (iFlag == JUMP_STATE_FLAG::JUMP_FROM_SLIDE)
            eJumpAni = m_bLeft ? ABILITY_ANI::SLIDE_JUMP_L : ABILITY_ANI::SLIDE_JUMP_R;
        else
            eJumpAni = m_bLeft ? ABILITY_ANI::JUMP_L : ABILITY_ANI::JUMP_R;

        pAbility->Play_AbilityAni(pKirby, eJumpAni);

        m_eJumpType = JUMP_STATE::JUMP;
    }

    // Ground Ignore
    m_fAccGroundIgnoreTime = m_fMaxGroundIgnoreTime;
}

void CKirby_Jump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    // 점프 직후 잠깐은 착지 판정 무시
    if (m_fAccGroundIgnoreTime > 0.f)
    {
        m_fAccGroundIgnoreTime -= fTimeDelta;
        m_fAccGroundIgnoreTime = (std::max)(m_fAccGroundIgnoreTime, 0.f);
    }

    CMovement_Child* pMovement = pKirby->Get_Movement();
    _bool bIsGround = pMovement->Is_Grounded();

    if (m_fAccGroundIgnoreTime <= 0.f && bIsGround == true)
    {
        //pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::FALL_DIRECT);
        Transition_Wait_OR_Run(pKirby);
        return;
    }

    if (bIsGround == true)
    {
        //pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::FALL_DIRECT);
        Transition_Wait_OR_Run(pKirby);
        return;
    }
   
    switch (m_eJumpType)
    {
        case JUMP_STATE::JUMP_START:
        {
            // Ani
            if (pKirby->Has_Deform())
            {
                CKirby_Deform* pKirbyDeform = pKirby->Get_KirbyDeform();
                CKirby_Deform_Model* pDeformModel = pKirby->Get_DeformPart_Model(pKirbyDeform->Get_DeformType());
                if (pDeformModel->Get_Animator()->Is_Finished())
                {
                    pKirbyDeform->Play_DeformAni(pKirby, DEFORM_ANI::JUMP);
                    m_eJumpType = JUMP_STATE::JUMP;
                }
            }
            else
            {
                MSG_BOX("Bug 1: CKirby_Jump");
            }
            break;
        }
        case JUMP_STATE::JUMP:
        {
            if (pMovement->Get_VerticalVelocity() > 0.f)
                break;

            if (pKirby->Has_Deform())
            {
                pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::FALL_DIRECT);
            }
            else
            {
                CKirby_Body* pKribyBody = pKirby->Get_Body();
                _bool bCanPlayJumpEnd = pKribyBody->Get_KirbyBody() != KIRBY_BODY_STATE::STUFFED;
                _bool bPlayJumpEnd = bCanPlayJumpEnd && (rand() % 2 == 0);

                CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
                _bool bCanPlayJumpEndFromSlide = pAbility->Can_PlayJumpEndFromSlide();

                if (bPlayJumpEnd && bCanPlayJumpEndFromSlide)
                {
                    if(m_bLeft)
                        pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::PLAY_JUMP_END_L);
                    else
                        pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::PLAY_JUMP_END_R);
                }
                else
                {
                    pKirby->Change_State(KIRBY_STATE_TYPE::FALL, FALL_STATE_FLAG::FALL_DIRECT);
                    return;
                }
            }
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
        {
            if (!pCommand->IsPress())
                return false;

            if (pKirby->Get_Movement()->Get_VerticalVelocity() <= 0.f &&
                Try_Transition_Ladder_CommandUp(pKirby))
                return true;

            Handle_MoveCommand(pKirby, pCommand);
            return true;
        }
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

            if (pKirby->Has_Deform())
                return true;

            if(pKirby->Get_Body()->Get_KirbyBody() != KIRBY_BODY_STATE::STUFFED)
                pKirby->Change_State(KIRBY_STATE_TYPE::HOVERING);

            return true;
        }
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            CKirby_AttackMode* pAttackMode = pKirby->Get_ActiveAttackMode();
            if (pAttackMode->Can_Attack(KIRBY_ATTACK_LOCATION::AIR))
            {
                if (pCommand->IsDown())
                    pAttackMode->Enter_Attack_KeyDown(pKirby);
                else if (pCommand->IsPress())
                    pAttackMode->Enter_Attack_KeyPress(pKirby);
                else if (pCommand->IsUp())
                    pAttackMode->Enter_Attack_KeyUp(pKirby);
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
