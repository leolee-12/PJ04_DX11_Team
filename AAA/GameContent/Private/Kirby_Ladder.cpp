#include "Kirby_Ladder.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

#include "Movement_Child.h"
#include "LevelDesign_Ladder.h"

CKirby_Ladder::CKirby_Ladder()
{
}

HRESULT CKirby_Ladder::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Ladder::Get_StateType()
{
    return KIRBY_STATE_TYPE::LADDER;
}

void CKirby_Ladder::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);
    
    //pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);
    //pKirby->Get_Body()->Get_Animator()->Play("LadderWait", true, false, 0.1f, 1.5f);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(false);

    CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();

    CTransform* pTransform = pKirby->Get_Transform();
    _vector vNearestSellPos = pLadder->Get_NearestRungWorld(pTransform->Get_State(STATE::POSITION));

    pTransform->Set_State(STATE::POSITION, vNearestSellPos);

    pMovement->Sync_To_Controller();
}

void CKirby_Ladder::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CMovement_Child* pMovement = pKirby->Get_Movement();

    constexpr _float fLadderSpeed = { 8.f };
    pMovement->Set_VelocityY(m_fClimbUpDown * fLadderSpeed);
    m_fClimbUpDown = 0;

    if (pKirby->IsLadder() == false)
        pKirby->Change_State(KIRBY_STATE_TYPE::FALL);
}

void CKirby_Ladder::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(true);
}

_bool CKirby_Ladder::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Top
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_pGameInstance_Proxy->Key_Pressing(DIK_F))
            {
                m_fClimbUpDown = 1.f;
            }
            return true;
        }
        // Down
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_pGameInstance_Proxy->Key_Pressing(DIK_F))
            {
                m_fClimbUpDown = -1.f;
            }
            return true;
        }
        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            pKirby->Change_State(KIRBY_STATE_TYPE::JUMP);
            return true;
        }
    }

    return false;
}

void CKirby_Ladder::Change_LadderState(CKirby* pKirby, LADDER_STATE eNext)
{
    if (m_eLadderState == eNext)
        return;

    Exit_LadderState(pKirby, m_eLadderState);

    m_eLadderState = eNext;

    Enter_LadderState(pKirby, m_eLadderState);
}

void CKirby_Ladder::Enter_LadderState(CKirby* pKirby, LADDER_STATE eState)
{
    switch (m_eLadderState)
    {
        case LADDER_STATE::WAIT:
            break;
        case LADDER_STATE::MOVE:
            break;
    }
}

void CKirby_Ladder::Update_LadderState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eLadderState)
    {
        case LADDER_STATE::WAIT:
            break;
        case LADDER_STATE::MOVE:
            break;
    }
}

void CKirby_Ladder::Exit_LadderState(CKirby* pKirby, LADDER_STATE eState)
{
    switch (m_eLadderState)
    {
        case LADDER_STATE::WAIT:
            break;
        case LADDER_STATE::MOVE:
            break;
    }
}

CKirby_Ladder* CKirby_Ladder::Create()
{
    CKirby_Ladder* pInstance = new CKirby_Ladder();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ladder");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ladder::Free()
{
    __super::Free();
}
