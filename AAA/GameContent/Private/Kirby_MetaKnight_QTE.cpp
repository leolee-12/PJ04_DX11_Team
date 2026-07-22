#include "Kirby_MetaKnight_QTE.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

#include "Boss_Metaknight.h"

namespace
{
    constexpr _float QTE_START_PROGRESS = 0.2f;
    constexpr _float QTE_INCREASE_PER_INPUT = 0.05f;
    constexpr _float QTE_DECREASE_PER_SEC = 0.06f;
    constexpr _float QTE_PROGRESS_FOLLOW_SPEED = 6.f;
    constexpr _float QTE_SUCCESS_PROGRESS = 0.9f;
    constexpr _float QTE_FAIL_PROGRESS = 0.005f;
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

    pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);

    m_eMetaKnightQTEState = METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END;
}

void CKirby_MetaKnight_QTE::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_MetaKnightState(pKirby, fTimeDelta);
}

void CKirby_MetaKnight_QTE::Exit(CKirby* pKirby)
{
    m_pGameInstance_Proxy->UnSubscribe(m_tQTEShowHandle);

    ENEMY_ATTACHMENT_END_DESC tDesc{};
    tDesc.eContext = ENEMY_ATTACHMENT_CONTEXT::METAKNIGHT_QTE;
    m_pGameInstance_Proxy->Publish(EventTag::Enemy_AttachmentEnd, &tDesc);

    m_eMetaKnightQTEState = METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END;

    m_bQTEStarted = false;
    m_pMetaKnight = nullptr;

    __super::Exit(pKirby);
}

_bool CKirby_MetaKnight_QTE::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    switch (pCommand->GetCommandType())
    {
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eMetaKnightQTEState != METAKNIGHT_QTE_STATE::QTE || !m_bQTEStarted)
                return true;

            m_fQTEProgress += QTE_INCREASE_PER_INPUT;
            Helper::FloatClamp(m_fQTEProgress, 0.f, 1.f);
            return true;
        }
    }
    return false;
}

void CKirby_MetaKnight_QTE::Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_LOCKING_WIN:
        {
            CTransform* pTransform = pKirby->Get_Transform();
            pTransform->Set_WorldMatrix(pDesc->AnchorWorld);
            pKirby->Get_Movement()->Sync_To_Controller();

            pAnimator->Play("Metaknight_DemoLockingSwordWinCut1", false, true, pDesc->fBlendDuration, pDesc->fAnimSpeed);

            Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::SUCCESS);
            break;
        }
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_UPPERCALIBUR:
        {
            CTransform* pTransform = pKirby->Get_Transform();
            pTransform->Set_WorldMatrix(pDesc->AnchorWorld);
            pKirby->Get_Movement()->Sync_To_Controller();

            CAnimator::ANI_PLAY_INFO tInfo{};
            tInfo.bLoop = false;
            tInfo.bRestart = true;
            tInfo.fBlend = pDesc->fBlendDuration;
            tInfo.fSpeed = pDesc->fAnimSpeed;

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut1";               pAnimator->Play(&tInfo);
            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut2";               pAnimator->Enqueue(tInfo);
            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut3";               pAnimator->Enqueue(tInfo);
            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut4";               pAnimator->Enqueue(tInfo);
            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut5";               pAnimator->Enqueue(tInfo);
            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut6";               pAnimator->Enqueue(tInfo);
            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut7";               pAnimator->Enqueue(tInfo);

            Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::FAIL);
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
        case KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_UPPERCALIBUR_END:
        case KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_LOCKING_WIN_END:
            break;
        default:
            MSG_BOX("Event Error 2: CKirby_MetaKnight_QTE");
            break;
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
            m_fQTEProgress = QTE_START_PROGRESS;
            m_fQTEAnimationProgress = QTE_START_PROGRESS;
            m_bQTEStarted = false;
            m_pMetaKnight = nullptr;

            pAnimator->Play("Metaknight_LockingSword", false, true, 0.f, 0.f);
            pAnimator->Seek(m_fQTEAnimationProgress);

            m_tQTEShowHandle = m_pGameInstance_Proxy->Subscribe(EventTag::QTE_Show, [this, pKirby](void*)
                {
                    if (m_eMetaKnightQTEState != METAKNIGHT_QTE_STATE::QTE)
                        return;

                    // MetaKnight 포인터 가지고 오기
                    BOSS_QUERY tQuery{};
                    m_pGameInstance_Proxy->Publish(EventTag::Query_Boss, &tQuery);
                    m_pMetaKnight = dynamic_cast<CBoss_Metaknight*>(tQuery.pBoss);

                    m_bQTEStarted = true;
                }
            );

            ENEMY_ATTACHMENT_BEGIN_DESC tDesc{};
            tDesc.pBoneMatrix = pKirby->Get_Body()->Get_BoneMatrixPtr("limbsM");
            tDesc.pAnchorWorld = pKirby->Get_Transform()->Get_WorldMatrixPtr();
            tDesc.eContext = ENEMY_ATTACHMENT_CONTEXT::METAKNIGHT_QTE;
            m_pGameInstance_Proxy->Publish(EventTag::Enemy_AttachmentBegin, &tDesc);
            break;
        }

        case METAKNIGHT_QTE_STATE::SUCCESS:
        {
            break;
        }
        case METAKNIGHT_QTE_STATE::FAIL:
        {
            break;
        }
        case METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END:
        {
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
            if (!m_bQTEStarted)
                break;

            m_fQTEProgress -= QTE_DECREASE_PER_SEC * fTimeDelta;
            Helper::FloatClamp(m_fQTEProgress, 0.f, 1.f);

            _float fFollowRatio = QTE_PROGRESS_FOLLOW_SPEED * fTimeDelta;
            Helper::FloatClamp(fFollowRatio, 0.f, 1.f);
            m_fQTEAnimationProgress += (m_fQTEProgress - m_fQTEAnimationProgress) * fFollowRatio;

            pAnimator->Seek(m_fQTEAnimationProgress);

            if (m_pMetaKnight != nullptr)
                m_pMetaKnight->Sync_LockingProgress(m_fQTEAnimationProgress);

            if (m_fQTEAnimationProgress >= QTE_SUCCESS_PROGRESS)
            {
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::SUCCESS);
                return;
            }

            if (m_fQTEAnimationProgress <= QTE_FAIL_PROGRESS)
            {
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::FAIL);
                return;
            }
            break;
        }

        case METAKNIGHT_QTE_STATE::SUCCESS:
            if (pAnimator->Is_Finished())
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END);
            break;
        case METAKNIGHT_QTE_STATE::FAIL:
            if (pAnimator->Get_CurrentAnimName() == "Metaknight_DemoUpperCaliburCut7" && pAnimator->Is_Finished())
                Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END);
            break;

        case METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END:
            Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
            break;
    }
}

void CKirby_MetaKnight_QTE::Exit_MetaKnightState(CKirby* pKirby, METAKNIGHT_QTE_STATE eState)
{
    switch (eState)
    {
        case METAKNIGHT_QTE_STATE::QTE:
            m_bQTEStarted = false;
            break;

        case METAKNIGHT_QTE_STATE::SUCCESS:
            break;
        case METAKNIGHT_QTE_STATE::FAIL:
        {
            CTransform* pTransform = pKirby->Get_Transform();
            CKirby_Body* pBody = pKirby->Get_Body();

            const _float4x4* pBone = pBody->Get_BoneMatrixPtr("TopL");
            if (pBone == nullptr)
                return;

            _matrix matBoneWorld = XMLoadFloat4x4(pBone) * XMLoadFloat4x4(pBody->Get_CombinedWorldMatrixPtr());
            _vector vBoneWorldPos = XMVectorSetW(matBoneWorld.r[3], 1.f);

            _vector vCurPos = pTransform->Get_State(STATE::POSITION);
            _vector vNewLook = -XMVector3Normalize(XMVectorSetY(pTransform->Get_State(STATE::RIGHT), 0.f));

            _vector vNewBoneWorldPos = XMVectorSetY(vBoneWorldPos, XMVectorGetY(vCurPos));

            pTransform->Set_State(STATE::POSITION, vNewBoneWorldPos);
            pTransform->LookTo(vNewLook);
            pKirby->Get_Movement()->Sync_To_Controller();
            break;
        }
        case METAKNIGHT_QTE_STATE::METANIGHT_QTE_STATE_END:
            break;
    }
}

void CKirby_MetaKnight_QTE::Request_MetaKnight_ParryBegin(CKirby* pKirby)
{
    Change_MetaKnightState(pKirby, METAKNIGHT_QTE_STATE::QTE);
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