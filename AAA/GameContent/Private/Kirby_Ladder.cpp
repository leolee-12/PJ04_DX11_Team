#include "Kirby_Ladder.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"
#include "LevelDesign_Ladder.h"

#include "Kirby_Jump.h"

#include "Effect_Loader.h"

CKirby_Ladder::CKirby_Ladder()
{
}

HRESULT CKirby_Ladder::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_iMaxDownAniCells = 2;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Ladder::Get_StateType()
{
    return KIRBY_STATE_TYPE::LADDER;
}

void CKirby_Ladder::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);
    
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby);

    pKirby->Put_WeaponOnBack(true);

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

        _vector vLadderPos = pLadder->Get_Transform()->Get_State(STATE::POSITION);
        vLadderPos = XMVectorSetY(vLadderPos, XMVectorGetY(vLadderCellPos));
        pTransform->LookAt(vLadderPos);

        pMovement->Sync_To_Controller();
    }
    else
    {
        MSG_BOX("Enter Bug: CKirby_Ladder");
    }

    // 변수 초기화
    m_iCurMoveDir = 0;
    m_iPreMoveDir = 0;
    m_iRemainDownAniCells = m_iMaxDownAniCells;

    m_eLadderState = LADDER_STATE::LADDER_END;
    Change_LadderState(pKirby, LADDER_STATE::WAIT);

    CEffect_Loader::GetInstance()->Spawn(L"OnLadderEffect", pKirby->Get_LevelIndex(),
        _float3{ 0.f, -0.2f, 0.f }, _float3{ 0.f, 0.f, 0.f }, _float3{ 0.f, 0.f, 0.f },
        pKirby->Get_Transform()->Get_WorldMatrixPtr());
}

void CKirby_Ladder::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
    if (pLadder == nullptr && m_eLadderState != LADDER_TOP_JUMP)
    {
        MSG_BOX("Update Bug Point 1: CKirby_Ladder");
        Transition_Fall_OR_Wait_OR_Run(pKirby);
        return;
    }

    Update_LadderState(pKirby, fTimeDelta);

    // 예약 x 조작감 별로
    m_iCurMoveDir = 0;
}

void CKirby_Ladder::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Stop();
    pMovement->Set_UseGravity(true);
    pMovement->Set_LinearDrag(CKirby::s_fLinearDrag);

    pKirby->Put_WeaponOnBack(false);
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
            if(m_eLadderState != LADDER_STATE::LADDER_TOP_JUMP)
                m_iCurMoveDir = 1;

            return true;
        }

        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_eLadderState != LADDER_STATE::LADDER_TOP_JUMP)
                m_iCurMoveDir = -1;

            return true;
        }

        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            pKirby->Change_State(KIRBY_STATE_TYPE::JUMP, JUMP_STATE_FLAG::FORCE_JUMP);
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
    switch (eState)
    {
        case LADDER_STATE::WAIT:
        {
            pKirby->Get_Body()->Get_Animator()->Play("LadderWait", true, false, 0.1f, 1.5f);
            m_iRemainDownAniCells = m_iMaxDownAniCells;
            break;
        }
        case LADDER_STATE::MOVE:
        {
            Apply_LadderUp(pKirby);
            break;
        }
        case LADDER_STATE::LADDER_TOP_JUMP:
        {
            pKirby->Put_WeaponOnBack(false);

            pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::JUMP_END_L);
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_LinearDrag(CKirby::s_fLinearDrag);
            pMovement->Force_Jump(3.f);

            break;
        }
    }
}

void CKirby_Ladder::Update_LadderState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eLadderState)
    {
        case LADDER_STATE::WAIT:
        {
            CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
            if(Handle_LadderTopBottom(pKirby, pLadder))
                return;

            if (m_iCurMoveDir != 0)
            {
                Set_NextCell();
                Change_LadderState(pKirby, LADDER_STATE::MOVE);
            }

            break;
        }
        case LADDER_STATE::MOVE:
        {
            if (m_iCurMoveDir != 0 && m_iPreMoveDir != m_iCurMoveDir)
            {
                std::swap(m_iCurLadderIndex, m_iNextLadderIndex);
                m_iPreMoveDir = m_iCurMoveDir;
                m_iCurMoveDir = 0;
            }

            _bool bMoveDown = m_iNextLadderIndex < m_iCurLadderIndex;
            if(m_bPlayAniLadderUp && bMoveDown && m_iRemainDownAniCells <= 0)
                Apply_LadderDown(pKirby);
            else if(!m_bPlayAniLadderUp && !bMoveDown)
                Apply_LadderUp(pKirby);

            CLevelDesign_Ladder* pLadder = pKirby->Get_Ladder();
            if (m_iCurLadderIndex != m_iNextLadderIndex)
            {
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
                const _float fPredictDistY = m_fLadderSpeed * fTimeDelta;

                _bool bWillArrive = fPredictDistY >= fRemainDistY;

                CMovement_Child* pMovement = pKirby->Get_Movement();

                if (bWillArrive)
                {
                    pTransform->Set_State(STATE::POSITION, vLadderNextCellPos);
                    pMovement->Sync_To_Controller();

                    if (bMoveDown)
                        --m_iRemainDownAniCells;
                    else
                        m_iRemainDownAniCells = m_iMaxDownAniCells;

                    m_iCurLadderIndex = m_iNextLadderIndex;

                    if (Handle_LadderTopBottom(pKirby, pLadder))
                        return;

                    if(m_iCurMoveDir == 0)
                    {
                        m_iPreMoveDir = 0;
                        Change_LadderState(pKirby, LADDER_STATE::WAIT);
                    }
                    else
                    {
                        Set_NextCell();
                    }
                    return;
                }

                _bool bMoveUp = m_iCurLadderIndex < m_iNextLadderIndex;
                if (bMoveUp)
                    pMovement->Set_VelocityY(m_fLadderSpeed);
                else
                    pMovement->Set_VelocityY(-m_fLadderSpeed);
            }
            break;
        }
        case LADDER_STATE::LADDER_TOP_JUMP:
        {
            _vector vLook = pKirby->Get_Transform()->Get_State(STATE::LOOK);
            vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

            CMovement_Child* pMovement = pKirby->Get_Movement();

            constexpr _float fSpeed = 5.f;
            pMovement->Set_VelocityX(XMVectorGetX(vLook) * fSpeed);
            pMovement->Set_VelocityZ(XMVectorGetZ(vLook) * fSpeed);

            if (pKirby->Get_Body()->Get_Animator()->Is_Finished())
                Transition_Wait_OR_Run(pKirby);
        }
    }
}

void CKirby_Ladder::Exit_LadderState(CKirby* pKirby, LADDER_STATE eState)
{
    switch (eState)
    {
        case LADDER_STATE::WAIT:
            break;
        case LADDER_STATE::MOVE:
            break;
    }
}

void CKirby_Ladder::Set_NextCell()
{
    m_iNextLadderIndex = m_iCurLadderIndex + m_iCurMoveDir;
    m_iPreMoveDir = m_iCurMoveDir;
    m_iCurMoveDir = 0;
}

_bool CKirby_Ladder::Handle_LadderTopBottom(CKirby* pKirby, CLevelDesign_Ladder* pLadder)
{
    if (m_iCurMoveDir == 1 && pLadder->Is_TopCell(m_iCurLadderIndex))
    {
        m_iCurMoveDir = 0;
        m_iPreMoveDir = 0;
        Change_LadderState(pKirby, LADDER_STATE::LADDER_TOP_JUMP);
        return true;
    }
    
    if (m_iCurMoveDir == -1 && pLadder->Is_BottomCell(m_iCurLadderIndex))
    {
        m_iCurMoveDir = 0;
        m_iPreMoveDir = 0;
        Transition_Fall_OR_Wait_OR_Run(pKirby);
        return true;
    }

    return false;
}

void CKirby_Ladder::Apply_LadderUp(CKirby* pKirby)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play("LadderUp", true, false, 0.1f, 1.5f);
    m_bPlayAniLadderUp = true;
    m_fLadderSpeed = 6.f;
}

void CKirby_Ladder::Apply_LadderDown(CKirby* pKirby)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pAnimator->Play("LadderDown", true, false, 0.1f, 1.5f);
    m_bPlayAniLadderUp = false;
    m_fLadderSpeed = 12.f;
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
