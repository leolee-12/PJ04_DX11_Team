#include "Kirby_Jump.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

 _bool CKirby_Jump::m_bLeft = false;


CKirby_Jump::CKirby_Jump()
{
}

HRESULT CKirby_Jump::Initialize()
{

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Jump::Get_StateType()
{
    return KIRBY_STATE_TYPE::JUMP;
}

void CKirby_Jump::Enter(CKirby* pKirby)
{
    CMovement* pMovementCom = pKirby->Get_Movement();
    pMovementCom->Jump();


    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    if (m_bLeft == true)
        pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::JUMP_L));
    else
        pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::JUMP_R));

    m_bFirstFrameSkip = false;
    m_eJumpType = JUMP_STATE::JUMP_STRAT;
}

void CKirby_Jump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    if (m_bFirstFrameSkip == false)
    {
        m_bFirstFrameSkip = true;
        return;
    }

    CMovement* pMovementCom = pKirby->Get_Movement();

    _float fYVelocity = pMovementCom->Get_VerticalVelocity();
    _bool bIsGround = pMovementCom->Is_Grounded();

    // Fall
    if (fYVelocity <= CKirby::s_fFallVelocityY)
    {
        pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
        if (rand() % 2 == 0)
        {
            CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

            if (m_bLeft == true)
                pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::JUMP_END_L));
            else
                pAnimator->Play(pKirby->Get_KirbyAbility()->Get_AniInfo(ABILITY_ANI::JUMP_END_R));
        }
    }      
}

void CKirby_Jump::Exit(CKirby* pKirby)
{
    m_bFirstFrameSkip = false;
    m_bLeft = !m_bLeft;
}

_bool CKirby_Jump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    __super::Handle_Command(pKirby, pCommand);

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Move
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
        // Jump
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            pKirby->Change_State(KIRBY_STATE_TYPE::HOVERING);
            return true;
        }
        // Attack Down
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsDown())
                return false;

            CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
            if(pAbility->Can_Attack(KIRBY_ATTACK_LOCATION::AIR))
                pAbility->Down_Attack(pKirby);
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
