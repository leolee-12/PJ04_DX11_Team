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
    pMovement->Set_LinearDrag(0.f);

    // 위치 Snap
    CTransform* pTransform = pKirby->Get_Transform();
    _vector vCurPos = pTransform->Get_State(STATE::POSITION);

    CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
    m_iCurLadderIndex = pLadder->Get_NearestCellIndex(vCurPos);
    m_iNextLadderIndex = m_iCurLadderIndex;

    _vector vLadderCellPos{};
    if (pLadder->Try_GetCellWorld(m_iCurLadderIndex, vLadderCellPos))
    {
        pTransform->Set_State(STATE::POSITION, vLadderCellPos);
        pMovement->Sync_To_Controller();
    }
    else
    {
        MSG_BOX("Enter Bug: CKirby_Ladder");
    }
}

void CKirby_Ladder::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
    if (pLadder == nullptr)
    {
        MSG_BOX("Update Bug Point 1: CKirby_Ladder");
        Transition_Fall_OR_Wait_OR_Run(pKirby);
        return;
    }

    if (m_iCurLadderIndex != m_iNextLadderIndex)
    {
        constexpr _float fLadderSpeed = 8.f;

        // 현재 위치
        CTransform* pTransform = pKirby->Get_Transform();
        _vector vCurPos = pTransform->Get_State(STATE::POSITION);

        // 목표 위치
        _vector vLadderNextCellPos{};
        if (!pLadder->Try_GetCellWorld(m_iNextLadderIndex, vLadderNextCellPos))
        {
            MSG_BOX("Update Bug Point 2: CKirby_Ladder");
            Transition_Fall_OR_Wait_OR_Run(pKirby);
            return;
        }

        const _float fCurrentY = XMVectorGetY(vCurPos);
        const _float fTargetY = XMVectorGetY(vLadderNextCellPos);

        const _float fRemainDistY = fabsf(fTargetY - fCurrentY);
        const _float fPredictDistY = fLadderSpeed * fTimeDelta;

        _bool bWillArrive = fPredictDistY >= fRemainDistY;

        CMovement_Child* pMovement = pKirby->Get_Movement();

        if (bWillArrive)
        {
            pTransform->Set_State(STATE::POSITION, vLadderNextCellPos);
            pMovement->Sync_To_Controller();

            m_iCurLadderIndex = m_iNextLadderIndex;
            return;
        }

        _bool bMoveUp = m_iCurLadderIndex < m_iNextLadderIndex;
        if (bMoveUp)
            pMovement->Set_VelocityY(fLadderSpeed);
        else
            pMovement->Set_VelocityY(-fLadderSpeed);
    }
}

void CKirby_Ladder::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(true);
    pMovement->Set_LinearDrag(CKirby::s_fLinearDrag);
}

_bool CKirby_Ladder::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        {
            if (!pCommand->IsPress())
                return false;

            CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
            if (pLadder->Is_TopCell(m_iCurLadderIndex))
            {
                Transition_Fall_OR_Wait_OR_Run(pKirby);
                return true;
            }

            m_iNextLadderIndex = m_iCurLadderIndex + 1;

            return true;
        }

        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (!pCommand->IsPress())
                return false;

            CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
            if (pLadder->Is_BottomCell(m_iCurLadderIndex))
            {
                Transition_Fall_OR_Wait_OR_Run(pKirby);
                return true;
            }

            m_iNextLadderIndex = m_iCurLadderIndex - 1;

            return true;
        }

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
