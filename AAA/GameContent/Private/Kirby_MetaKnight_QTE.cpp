#include "Kirby_MetaKnight_QTE.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

namespace
{
    constexpr _float QTE_START_PROGRESS = 0.2f;
    constexpr _float QTE_INCREASE_PER_INPUT = 0.06f;
    constexpr _float QTE_DECREASE_PER_SEC = 0.12f;
}

CKirby_MetaKnight_QTE::CKirby_MetaKnight_QTE()
{
}

HRESULT CKirby_MetaKnight_QTE::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_MetaKnight_QTE::Get_StateType()
{
    return KIRBY_STATE_TYPE::METAKNIGHT_QTE;
}

void CKirby_MetaKnight_QTE::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);

    m_iQTE_InputCount = 0;
    m_fQTEProgress = QTE_START_PROGRESS;

    // 메타나이트 부착 이벤트
    ENEMY_ATTACHMENT_BEGIN_DESC tDesc{};
    tDesc.pBoneMatrix = pKirby->Get_Body()->Get_BoneMatrixPtr("limbsM");
    tDesc.pAnchorWorld =pKirby->Get_Transform()->Get_WorldMatrixPtr();
    tDesc.eContext =ENEMY_ATTACHMENT_CONTEXT::METAKNIGHT_QTE;
    m_pGameInstance_Proxy->Publish(EventTag::Enemy_AttachmentBegin, &tDesc);

    m_eMetaKnightQTEState = METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END;
    Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::QTE);
}

void CKirby_MetaKnight_QTE::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_MetaKnightState(pKirby, fTimeDelta);
}

void CKirby_MetaKnight_QTE::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_MetaKnight_QTE::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Attack
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eMetaKnightQTEState != METAKNIGHT_QTE_STATE::QTE)
                return true;

            ++m_iQTE_InputCount;

            m_fQTEProgress += QTE_INCREASE_PER_INPUT;
            Helper::FloatClamp(m_fQTEProgress, 0.f, 1.f);

            return true;
        }
    }

    return false;
}

void CKirby_MetaKnight_QTE::Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_LOCKING:
        {
            // 위치
            CTransform* pTransform = pKirby->Get_Transform();
            pTransform->Set_WorldMatrix(pDesc->AnchorWorld);

            pKirby->Get_Movement()->Sync_To_Controller();

            // Ani
            CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
            pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);

            CAnimator::ANI_PLAY_INFO tInfo{};
            tInfo.bLoop = false;
            tInfo.bRestart = true;
            tInfo.fBlend = pDesc->fBlendDuration;
            tInfo.fSpeed = pDesc->fAnimSpeed;

            tInfo.strAniName = "Metaknight_LockingSword";
            pAnimator->Play(&tInfo);       
            break;
        }
        default:
        {
            MSG_BOX("Event Error 1: CKirby_MetaKnight_QTE");
            break;
        }
    }
}

void CKirby_MetaKnight_QTE::Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc)
{
    switch (pDesc->eType)
    {
    case KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_LOCKING_END:
    {
        // Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
        break;
    }
    default:
    {
        MSG_BOX("Event Error 2: CKirby_MetaKnight_QTE");
        break;
    }
    }
}

void CKirby_MetaKnight_QTE::Change_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eNext)
{
    if (m_eMetaKnightQTEState == eNext)
        return;

    Exit_MetaKnightState(pKirby, m_eMetaKnightQTEState);

    m_eMetaKnightQTEState = eNext;

    Enter_MetaKnightState(pKirby, m_eMetaKnightQTEState);
}

void CKirby_MetaKnight_QTE::Enter_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (eState)
    {
        case METAKNIGHT_QTE_STATE::QTE:
        {
            pAnimator->Play("Metaknight_LockingSword", false, true, 0.1f, 0.f);
            pAnimator->Seek(m_fQTEProgress);
            break;
        }
        case METAKNIGHT_QTE_STATE::SUCCESS:
        {
            pAnimator->Play("Metaknight_DemoLockingSwordWinCut1", false, true, 0.1f, 1.5f);
            break;
        }
        case METAKNIGHT_QTE_STATE::FAIL:
        {
            MSG_BOX("Not implemented: CKirby_MetaKnight_QTE");
            Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END);
            break;
        }
    }
}

void CKirby_MetaKnight_QTE::Update_MetaKnightState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eMetaKnightQTEState)
    {
        case METAKNIGHT_QTE_STATE::QTE:
        {
            if (m_fQTEProgress >= 1.f)
            {
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::SUCCESS);
                return;
            }

            m_fQTEProgress -= QTE_DECREASE_PER_SEC * fTimeDelta;
            Helper::FloatClamp(m_fQTEProgress, 0.f, 1.f);

            pAnimator->Seek(m_fQTEProgress);

            if (m_fQTEProgress <= 0.f)
            {
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::FAIL);
                return;
            }

            break;
        }
        case METAKNIGHT_QTE_STATE::SUCCESS:
        case METAKNIGHT_QTE_STATE::FAIL:
        {
            if (pAnimator->Is_Finished())
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END);
            break;
        }
        case METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END:
        {
            Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
            break;
        }
    }
}

void CKirby_MetaKnight_QTE::Exit_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eState)
{
    switch (eState)
    {
    case METAKNIGHT_QTE_STATE::QTE:
        break;
    case METAKNIGHT_QTE_STATE::SUCCESS:
        break;
    case METAKNIGHT_QTE_STATE::FAIL:
        break;
    }
}

void CKirby_MetaKnight_QTE::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

CKirby_MetaKnight_QTE* CKirby_MetaKnight_QTE::Create()
{
    CKirby_MetaKnight_QTE* pInstance = new CKirby_MetaKnight_QTE();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_MetaKnight_QTE");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_MetaKnight_QTE::Free()
{
    __super::Free();
}