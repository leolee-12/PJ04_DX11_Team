#include "Kirby_Ability_Normal.h"

#include "GameInstance.h"
#include "Effect_Loader.h"

#include "Movement_Child.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Inhalable.h"
#include "Monster.h"
#include "VacuumContainer.h"

CKirby_Ability_Normal::CKirby_Ability_Normal()
{
}

HRESULT CKirby_Ability_Normal::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    m_fMaxSuperInhaleTime = 1.f;

    m_vInhaleEffectStartPos = { 0.f, 0.63f, 0.6f };
    m_vInhaleEffectEndPos = { 0.f, 0.93f, 0.4f };

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Normal::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::NORMAL;
}

void CKirby_Ability_Normal::Enter_Ability(CKirby* pKirby)
{
    m_eInhaleState = INHALE_STATE::NORMAL_EXIT;

    m_bReqEndAttackState = false;
    m_bReqEndInhale = false;

    m_fAccSuperInhaleTime = 0.f;

    m_eCurMoveState = INHALE_MOVE_STATE::WAIT;

    m_bSuperInhaleEffectRaised = false;

    switch (m_eMouthState)
    {
        case MOUTH_STATE::DEFAULT:
        {
            Subscribe_InhaleCapturedEvent(pKirby);

            CEffect_Loader::GetInstance()->Spawn(L"VacuumContainer", pKirby->Get_LevelIndex(),
                m_vInhaleEffectStartPos, _float3(0.f, 0.f, 1.f),
                pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pInhaleEffect);

            static_cast<CVacuumContainer*>(m_pInhaleEffect)->Off_SuperInhale();

            Change_InhaleState(pKirby, INHALE_STATE::INHALE_LOOP);

            break;
        }
        case MOUTH_STATE::STUFFFED:
        {
            pKirby->Spit_Inhalable();

            Change_InhaleState(pKirby, INHALE_STATE::STUFFED_SPIT);

            break;
        }
    }
}

ABILITY_UPDATE_RESULT CKirby_Ability_Normal::Update_Ability(CKirby* pKirby, _float fTimeDelta)
{
    Update_InhaleMoveState(pKirby);
    Update_SuperInhaleTimer(fTimeDelta);
    Update_InhaleState(pKirby, fTimeDelta);

    return ABILITY_UPDATE_RESULT::NONE;
}

void CKirby_Ability_Normal::Exit_Ability(CKirby* pKirby)
{
    switch (m_eMouthState)
    {
        case MOUTH_STATE::DEFAULT:
        {
            End_InhaleCollider(pKirby);
            Unsubscribe_InhaleCapturedEvent();

            Restore_KirbyAfterInhale(pKirby);
            break;
        }

        case MOUTH_STATE::STUFFFED:
            break;
    }
}

_bool CKirby_Ability_Normal::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
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

            Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
            pKirby->Add_MoveDir(pMoveCommand->Get_Dir());
            return true;
        }

        // Attack Up
        case KIRBY_COMMAND_TYPE::ATTACK:
        {
            if (!pCommand->IsUp())
                return false;

            if (m_eMouthState != MOUTH_STATE::DEFAULT)
                return true;

            if (m_eInhaleState == INHALE_STATE::INHALE_LOOP ||
                m_eInhaleState == INHALE_STATE::SUPER_INHALE_START ||
                m_eInhaleState == INHALE_STATE::SUPER_INHALE_LOOP)
            {
                m_bReqEndInhale = true;
            }

            m_bReqEndInhale = true;
            return true;
        }
    }

    return false;
}

_bool CKirby_Ability_Normal::Enter_Attack_KeyDown(CKirby* pKirby)
{
    pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);

    return true;
}

_bool CKirby_Ability_Normal::Enter_Attack_KeyPress(CKirby* pKirby)
{
    // 무시
    return true;
}

_bool CKirby_Ability_Normal::Enter_Attack_KeyUp(CKirby* pKirby)
{
    // 무시
    return true;
}

_bool CKirby_Ability_Normal::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
        case KIRBY_ATTACK_LOCATION::GROUND:     return true;
        case KIRBY_ATTACK_LOCATION::AIR:        return true;
    }

    return false;
}

void CKirby_Ability_Normal::Change_InhaleState(CKirby* pKirby, INHALE_STATE eNext)
{
    if (m_eInhaleState == eNext)
        return;

    Exit_InhaleState(pKirby, m_eInhaleState);

    m_eInhaleState = eNext;

    Enter_InhaleState(pKirby, m_eInhaleState);
}

void CKirby_Ability_Normal::Enter_InhaleState(CKirby* pKirby, INHALE_STATE eState)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (eState)
    {
        case INHALE_STATE::INHALE_LOOP:
        {
            pBody->Set_Body(KIRBY_BODY_STATE::INHALE);
            pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);

            pMovement->Set_MaxHorizontalSpeed(2.f);

            Start_InhaleCollider(pKirby);
            Play_InhaleLoopAnimation(pKirby);
            break;
        }

        case INHALE_STATE::SUPER_INHALE_START:
        {
            pBody->Set_Eye(KIRBY_EYE_STATE::ANGRY);
            pAnimator->Play("SuperInhaleStart", false, false, 0.1f, 2.5f);

            static_cast<CVacuumContainer*>(m_pInhaleEffect)->On_SuperInhale();
            break;
        }

        case INHALE_STATE::SUPER_INHALE_LOOP:
        {
            pBody->Set_Eye(KIRBY_EYE_STATE::CLOSE);
            Play_InhaleLoopAnimation(pKirby);
            break;
        }

        case INHALE_STATE::INHALE_END:
        {
            End_InhaleCollider(pKirby);

            if (m_pInhaleEffect)
            {
                m_pInhaleEffect->EffectContainer_Stop();
                m_pInhaleEffect = nullptr;
            }

            pAnimator->Play("InhaleEnd", false, false, 0.1f, 1.5f);
            break;
        }

        case INHALE_STATE::STUFFED_START:
        {
            pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);
            pAnimator->Play("Stuffed", false, false, 0.1f, 1.5f, true);
            pBody->Set_Body(KIRBY_BODY_STATE::STUFFED);
            break;
        }


        case INHALE_STATE::STUFFED_SPIT:
        {
            pAnimator->Play("Spit", false, false, 0.1f, 2.f, true);
            break;
        }


        case INHALE_STATE::NORMAL_EXIT:
        {
            m_bReqEndAttackState = true;
            break;
        }
    }
}

void CKirby_Ability_Normal::Update_InhaleState(CKirby* pKirby, _float fTimeDelta)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    if (m_bReqEndInhale &&
        (m_eInhaleState == INHALE_STATE::INHALE_LOOP ||
            m_eInhaleState == INHALE_STATE::SUPER_INHALE_START ||
            m_eInhaleState == INHALE_STATE::SUPER_INHALE_LOOP))
    {
        m_bReqEndInhale = false;
        Change_InhaleState(pKirby, INHALE_STATE::INHALE_END);
        return;
    }

    switch (m_eInhaleState)
    {
        case INHALE_STATE::INHALE_LOOP:
        {
            if (m_fAccSuperInhaleTime >= m_fMaxSuperInhaleTime)
                Change_InhaleState(pKirby, INHALE_STATE::SUPER_INHALE_START);
            break;
        }

        case INHALE_STATE::SUPER_INHALE_START:
        {
            _float fRatio = pAnimator->Get_Progress();
            Update_SuperInhaleEffectRise(fRatio);

            if (pAnimator->Is_Finished())
                Change_InhaleState(pKirby, INHALE_STATE::SUPER_INHALE_LOOP);
            break;
        }

        case INHALE_STATE::SUPER_INHALE_LOOP:
        {
            break;
        }

        case INHALE_STATE::INHALE_END:
        {
            if (pAnimator->Get_Progress() >= 0.5f)
                pBody->Set_Body(KIRBY_BODY_STATE::NORMAL);

            if (pAnimator->Is_Finished())
                Change_InhaleState(pKirby, INHALE_STATE::NORMAL_EXIT);

            break;
        }

        case INHALE_STATE::STUFFED_START:
        {
            if (pAnimator->Is_Finished())
                Change_InhaleState(pKirby, INHALE_STATE::NORMAL_EXIT);

            break;
        }


        case INHALE_STATE::STUFFED_SPIT:
        {
            _float fRatio = pAnimator->Get_Progress();

            if (fRatio >= 0.4f)
            {
                pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::NORMAL);
                Change_MouthState(MOUTH_STATE::DEFAULT);
            }
            else if (fRatio >= 0.1f)
            {
                pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::INHALE);
            }
            else
            {
                pKirby->Get_Body()->Set_Body(KIRBY_BODY_STATE::STUFFED);
            }

            if (pAnimator->Is_Finished())
                Change_InhaleState(pKirby, INHALE_STATE::NORMAL_EXIT);

            break;
        }


        case INHALE_STATE::NORMAL_EXIT:
        {
            m_bReqEndAttackState = true;
            break;
        }
    }
}

void CKirby_Ability_Normal::Exit_InhaleState(CKirby* pKirby, INHALE_STATE eState)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CMovement_Child* pMovement = pKirby->Get_Movement();

    switch (eState)
    {
        case INHALE_STATE::INHALE_LOOP:
        case INHALE_STATE::SUPER_INHALE_START:
        case INHALE_STATE::SUPER_INHALE_LOOP:
            break;

        case INHALE_STATE::INHALE_END:
            pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);
            pBody->Set_Body(KIRBY_BODY_STATE::NORMAL);

            pKirby->Get_Movement()->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            break;

        case INHALE_STATE::STUFFED_START:
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
            break;


        case INHALE_STATE::STUFFED_SPIT:
            break;


        case INHALE_STATE::NORMAL_EXIT:
            break;
    }
}

void CKirby_Ability_Normal::Change_MouthState(MOUTH_STATE eMouthState)
{
    if (m_eMouthState == eMouthState)
        return;

    m_eMouthState = eMouthState;

    switch (m_eMouthState)
    {
        case MOUTH_STATE::DEFAULT:
            Set_FullBodyAni(ABILITY_ANI::WAIT, "Wait", true, false, 0.1f, 1.8f);
            Set_FullBodyAni(ABILITY_ANI::RUN, "Run", true, false, 0.1f, 2.2f);
            Set_FullBodyAni(ABILITY_ANI::JUMP_L, "JumpL", false, false, 0.1f, 5.f);
            Set_FullBodyAni(ABILITY_ANI::JUMP_R, "JumpR", false, false, 0.1f, 5.f);
            Set_FullBodyAni(ABILITY_ANI::FALL, "Fall", false, false, 0.1f, 2.f);
            Set_FullBodyAni(ABILITY_ANI::LANDING, "Landing", false, false, 0.05f, 1.f);
            Set_FullBodyAni(ABILITY_ANI::DAMAGED, "Damage", false, false, 0.1f, 1.5f);
            break;

        case MOUTH_STATE::STUFFFED:
            Set_FullBodyAni(ABILITY_ANI::WAIT, "StuffedWait", true, false, 0.1f, 2.5f);
            Set_FullBodyAni(ABILITY_ANI::RUN, "StuffedRun", true, false, 0.1f, 3.f);
            Set_FullBodyAni(ABILITY_ANI::JUMP_L, "StuffedJump", false, false, 0.1f, 2.5f);
            Set_FullBodyAni(ABILITY_ANI::JUMP_R, "StuffedJump", false, false, 0.1f, 2.5f);
            Set_FullBodyAni(ABILITY_ANI::FALL, "StuffedFall", true, false, 0.1f, 1.5f);
            Set_FullBodyAni(ABILITY_ANI::LANDING, "StuffedLanding", false, false, 0.1f, 1.5f);
            Set_FullBodyAni(ABILITY_ANI::DAMAGED, "StuffedDamage", false, false, 0.1f, 1.5f);
            break;
    }
}

void CKirby_Ability_Normal::Update_InhaleMoveState(CKirby* pKirby)
{
    INHALE_MOVE_STATE eNextMoveState = INHALE_MOVE_STATE::WAIT;

    CMovement_Child* pMovement = pKirby->Get_Movement();

    if (pMovement->Is_Grounded() == false)
        eNextMoveState = INHALE_MOVE_STATE::FALL;
    else if (pKirby->Has_MoveDir() == true)
        eNextMoveState = INHALE_MOVE_STATE::WALK;

    if (m_eCurMoveState == eNextMoveState)
        return;

    m_eCurMoveState = eNextMoveState;

    if (m_eInhaleState == INHALE_STATE::SUPER_INHALE_LOOP &&
        m_eCurMoveState == INHALE_MOVE_STATE::WAIT)
    {
        Change_InhaleState(pKirby, INHALE_STATE::SUPER_INHALE_START);
        return;
    }

    if (m_eInhaleState == INHALE_STATE::INHALE_LOOP ||
        m_eInhaleState == INHALE_STATE::SUPER_INHALE_LOOP)
    {
        Play_InhaleLoopAnimation(pKirby);
    }
}

void CKirby_Ability_Normal::Play_InhaleLoopAnimation(CKirby* pKirby)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    _string strAniName;

    switch (m_eInhaleState)
    {
        case INHALE_STATE::INHALE_LOOP:
        {
            if (m_eCurMoveState == INHALE_MOVE_STATE::WAIT)
                strAniName = "Inhale";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::WALK)
                strAniName = "InhaleWalk";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::FALL)
                strAniName = "InhaleFall";
            break;
        }

        case INHALE_STATE::SUPER_INHALE_LOOP:
        {
            if (m_eCurMoveState == INHALE_MOVE_STATE::WAIT)
                strAniName = "SuperInhale";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::WALK)
                strAniName = "SuperInhaleWalk";
            else if (m_eCurMoveState == INHALE_MOVE_STATE::FALL)
                strAniName = "SuperInhaleFall";
            break;
        }
    }

    pAnimator->Play(strAniName, true, false, 0.05f, 1.5f);
}

void CKirby_Ability_Normal::Update_SuperInhaleTimer(_float fTimeDelta)
{
    if (m_eInhaleState != INHALE_STATE::INHALE_LOOP)
        return;

    if (m_fAccSuperInhaleTime < m_fMaxSuperInhaleTime)
        m_fAccSuperInhaleTime += fTimeDelta;
}

void CKirby_Ability_Normal::Restore_KirbyAfterInhale(CKirby* pKirby)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    CKirby_Body* pBody = pKirby->Get_Body();
    pBody->Set_Eye(KIRBY_EYE_STATE::IDLE);
    pBody->Set_Body(KIRBY_BODY_STATE::NORMAL);
}

void CKirby_Ability_Normal::Start_InhaleCollider(CKirby* pKirby)
{
    CCollider* pInhaleCollider = pKirby->Get_Collider(CKirby::KIRBY_COLLIDER::INHALE_BOX);

    pInhaleCollider->Set_OnStay
    (
        [this, pKirby](CCollider* pOtherCollider)
        {
            IInhalable* pInhalableTarget = dynamic_cast<IInhalable*>(pOtherCollider->Get_Owner());
            if (pInhalableTarget == nullptr)
                return;

            _bool bIsSuperInhale =
                m_eInhaleState == INHALE_STATE::SUPER_INHALE_START ||
                m_eInhaleState == INHALE_STATE::SUPER_INHALE_LOOP;

            INHALE_QUERY tInhaleQuery{ bIsSuperInhale, pKirby };

            if (pInhalableTarget->Can_BeInhaled(tInhaleQuery))
                pInhalableTarget->Be_Captured(pKirby);
        }
    );

    pInhaleCollider->Set_Enabled(true);
}

void CKirby_Ability_Normal::End_InhaleCollider(CKirby* pKirby)
{
    CCollider* pInhaleBox = pKirby->Get_Collider(CKirby::KIRBY_COLLIDER::INHALE_BOX);
    pInhaleBox->Set_Enabled(false);
    pInhaleBox->Set_OnStay(nullptr);
}

void CKirby_Ability_Normal::Subscribe_InhaleCapturedEvent(CKirby* pKirby)
{
    if (m_bSubscribedInhaleCapturedEvent == true)
        return;

    m_hInhaleCapturedEvent = m_pGameInstance_Proxy->Subscribe(
        EVT_SWALLOWED,
        [this, pKirby](void* pData)
        {
            SWALLOW_EVENT* pEvent = static_cast<SWALLOW_EVENT*>(pData);
            if (pEvent == nullptr || pEvent->pInhalable == nullptr)
                return;

            Handle_InhaleCaptured(pKirby, pEvent->pInhalable);
        }
    );

    m_bSubscribedInhaleCapturedEvent = true;
}

void CKirby_Ability_Normal::Unsubscribe_InhaleCapturedEvent()
{
    if (m_bSubscribedInhaleCapturedEvent == false)
        return;

    m_pGameInstance_Proxy->UnSubscribe(m_hInhaleCapturedEvent);

    m_bSubscribedInhaleCapturedEvent = false;
}

void CKirby_Ability_Normal::Handle_InhaleCaptured(CKirby* pKirby, IInhalable* pInhaleable)
{
    End_InhaleCollider(pKirby);
    Unsubscribe_InhaleCapturedEvent();

    COPY_ABILITY_TYPE eAbility = pInhaleable->Get_CopyAbility();

    if (m_pInhaleEffect)
    {
        m_pInhaleEffect->EffectContainer_Stop();
        m_pInhaleEffect = nullptr;
    }

    if (eAbility == COPY_ABILITY_TYPE::NONE || eAbility == COPY_ABILITY_TYPE::NORMAL)
    {
        pKirby->Capture_Inhalable(pInhaleable);

        Change_MouthState(MOUTH_STATE::STUFFFED);
        Change_InhaleState(pKirby, INHALE_STATE::STUFFED_START);
    }
    else
    {
        pKirby->Request_ChangeKirbyAbility(eAbility);
        pKirby->Change_State(KIRBY_STATE_TYPE::GET_ABILITY);
    }
}

void CKirby_Ability_Normal::Update_SuperInhaleEffectRise(_float fRatio)
{
    if (m_bSuperInhaleEffectRaised)
        return;

    _vector vInhaleEffectStartPos = XMLoadFloat3(&m_vInhaleEffectStartPos);
    _vector vInhaleEffectEndPos = XMLoadFloat3(&m_vInhaleEffectEndPos);

    _vector vCurPos;

    if (fRatio >= 1.f)
    {
        m_bSuperInhaleEffectRaised = true;
        vCurPos = vInhaleEffectEndPos;
    }
    else
    {
        _float fEaseRatio = 1.f - powf(1.f - fRatio, 3.f);
        vCurPos = vInhaleEffectStartPos + (vInhaleEffectEndPos - vInhaleEffectStartPos) * fEaseRatio;
    }

    vCurPos = XMVectorSetW(vCurPos, 1.f);
    m_pInhaleEffect->Get_Transform()->Set_State(STATE::POSITION, vCurPos);
}

CKirby_Ability_Normal* CKirby_Ability_Normal::Create()
{
    CKirby_Ability_Normal* pInstance = new CKirby_Ability_Normal();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Normal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Normal::Free()
{
    Unsubscribe_InhaleCapturedEvent();

    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}