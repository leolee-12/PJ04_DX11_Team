#include "Kirby_Ladder.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_OnOffPart.h"
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

    pKirby->Set_OnOffPartMode(KIRBY_PART_MODE::BACK);

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
    m_fLadderAnimProgress = 0.f;

    m_eLadderState = LADDER_STATE::LADDER_END;
    Change_LadderState(pKirby, LADDER_STATE::WAIT);

    CEffect_Loader::GetInstance()->Spawn(L"OnLadderEffect", pKirby->Get_LevelIndex(),
        _float3{ 0.f, -0.2f, 0.f }, _float3{ 0.f, 0.f, 0.f }, _float3{ 0.f, 0.f, 0.f },
        pKirby->Get_RenderWorldMatrixPtr());

    m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_Ladder.wav", 0.5f);
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

    pKirby->Set_OnOffPartMode(KIRBY_PART_MODE::DEFAULT);
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
            pKirby->Set_OnOffPartMode(KIRBY_PART_MODE::DEFAULT);

            pKirby->Get_KirbyAbility()->Play_AbilityAni(pKirby, ABILITY_ANI::JUMP_END_L);
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_LinearDrag(CKirby::s_fLinearDrag);
            pMovement->Force_Jump(3.2f);

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
            // 사다리 이동 중에 방향이 바뀌면
            if (m_iCurMoveDir != 0 && m_iPreMoveDir != m_iCurMoveDir)
            {
                // 현재 위치와 목표 위치를 변경
                std::swap(m_iCurLadderIndex, m_iNextLadderIndex);
                m_iPreMoveDir = m_iCurMoveDir;
                m_iCurMoveDir = 0;
            }

            // 애니메이션 변경, 아래로 내려갈 때는 m_iRemainDownAniCells 소모해야 Ani 전환
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

                // Up Animation인 경우 사다리 올라간 비율에 맞게 Ani 설정
                if (m_bPlayAniLadderUp)
                {
                    // 시작 셀의 위치를 가지고 온다.
                    _vector vCurrentCellPos{};
                    if (pLadder->Try_GetCellWorld(m_iCurLadderIndex, vCurrentCellPos))
                    {
                        const _float fStartY = XMVectorGetY(vCurrentCellPos);
                        const _float fTotalDistance = fabsf(fTargetY - fStartY);

                        if (fTotalDistance <= Helper::fEpsilon) MSG_BOX("Update Bug Point 3: CKirby_Ladder");

                        // 비율을 구해서
                        _float fMoveRatio = fabsf(fCurrentY - fStartY) / fTotalDistance;
                        Helper::FloatClamp(fMoveRatio, 0.f, 1.f);

                        // 애니메이션에 적용
                        constexpr _float fMid = 0.5f;
                        const _float fAnimProgress = fmodf(m_fLadderAnimProgress + fMoveRatio * fMid, 1.f);
                        pKirby->Get_Body()->Get_Animator()->Seek(fAnimProgress);
                    }
                }

                CMovement_Child* pMovement = pKirby->Get_Movement();

                // 남은 거리보다 이동할 거리가 크면(도착)
                const _float fRemainDistY = fabsf(fTargetY - fCurrentY);
                const _float fPredictDistY = m_fLadderSpeed * fTimeDelta;

                // 도착 정리
                _bool bWillArrive = fPredictDistY >= fRemainDistY;
                if (bWillArrive)
                {
                    if(m_bPlayAniLadderUp)
                        m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_Ladder.wav", 0.5f);

                    // Snap 시키고
                    pTransform->Set_State(STATE::POSITION, vLadderNextCellPos);
                    pMovement->Sync_To_Controller();

                    // 아래로 내려가는 중이였다면 Down Ani 전환을 위한 셀 개수 감소
                    if (bMoveDown)
                        --m_iRemainDownAniCells;
                    else // 올라가는 중이였으면 초기화
                        m_iRemainDownAniCells = m_iMaxDownAniCells;

                    // 올라가는 중이었으면
                    if (m_bPlayAniLadderUp)
                    {
                        // 진행 비율에 맞게 올라가는 애니메이션 세팅
                        constexpr _float fMid = 0.5f;
                        m_fLadderAnimProgress = fmodf(m_fLadderAnimProgress + fMid, 1.f);
                        pKirby->Get_Body()->Get_Animator()->Seek(m_fLadderAnimProgress);
                    }
                    
                    m_iCurLadderIndex = m_iNextLadderIndex;

                    // 최상단, 최하단 검사후 상태 전이 처리
                    if (Handle_LadderTopBottom(pKirby, pLadder))
                        return;

                    // 입력이 더 없었으면 Wait
                    if(m_iCurMoveDir == 0)
                    {
                        m_iPreMoveDir = 0;
                        Change_LadderState(pKirby, LADDER_STATE::WAIT);
                    }
                    else // 있으면 다음 목표 세팅
                    {
                        Set_NextCell();
                    }
                    return;
                }

                // 속도 설정
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
    // 애니 전환만하고
    pAnimator->Play("LadderUp", true, false, 0.1f, 0.f);
    // 비율에 맞게 세팅
    pAnimator->Seek(m_fLadderAnimProgress);
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
