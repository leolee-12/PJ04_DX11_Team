#include "Kirby_Ability_Sleep.h"

#include "Kirby.h"
#include "Kirby_Body.h"

#include "Movement_Child.h"

CKirby_Ability_Sleep::CKirby_Ability_Sleep()
{
}

HRESULT CKirby_Ability_Sleep::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_wstrAttackModeName = L"½½¸³";

    return S_OK;
}

COPY_ABILITY_TYPE CKirby_Ability_Sleep::Get_AbilityType()
{
    return COPY_ABILITY_TYPE::SLEEP;
}

void CKirby_Ability_Sleep::Enter_AttackState(CKirby* pKirby, _int iFlag)
{
    pKirby->Get_Movement()->Set_MaxHorizontalSpeed(2.f);

    m_bReqEndAttackState = false;
    m_fAccSleepBeforeTime = 3.f;
    m_vMoveDir = {};
    m_iSleepAniCount = 4;

    m_eSleepState = SLEEP_STATE::SLEEP_STATE_END;
    m_eSleepBeforeState = SLEEP_BEFORE_STATE::SLEEP_BEFORE_STATE_END;
    Change_SleepState(pKirby, SLEEP_STATE::SLEEP_WAIT_START);
}

void CKirby_Ability_Sleep::Update_AttackState(CKirby* pKirby, _float fTimeDelta)
{
    Update_SleepState(pKirby, fTimeDelta);
}

void CKirby_Ability_Sleep::Exit_AttackState(CKirby* pKirby)
{
    pKirby->Get_Movement()->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

    m_eSleepState = SLEEP_STATE::SLEEP_STATE_END;
    m_eSleepBeforeState = SLEEP_BEFORE_STATE::SLEEP_BEFORE_STATE_END;

    pKirby->Set_AbilityPartsActive(COPY_ABILITY_TYPE::SLEEP, false);
    pKirby->Request_ChangeKirbyAbility(COPY_ABILITY_TYPE::NORMAL);
    pKirby->Apply_ChangeKirbyAbility();

    KIRBY_NAME_UPDATED tNameDesc{};
    tNameDesc.strAtkModeName = pKirby->Get_KirbyAbility()->Get_AttackModeName();
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tNameDesc);
}

_bool CKirby_Ability_Sleep::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (pCommand->GetCommandType())
    {
        case KIRBY_COMMAND_TYPE::MOVE_TOP:
        case KIRBY_COMMAND_TYPE::MOVE_DOWN:
        case KIRBY_COMMAND_TYPE::MOVE_LEFT:
        case KIRBY_COMMAND_TYPE::MOVE_RIGHT:
        {
            if (!pCommand->IsPress())
                return false;

            if (m_eSleepState == SLEEP_STATE::SLEEP_WAIT_START ||
                m_eSleepState == SLEEP_STATE::SLEEP_MOVE_ACTIVE)
            {
                Move_Command* pMoveCommand = static_cast<Move_Command*>(pCommand);
                XMStoreFloat3(&m_vMoveDir,
                    XMVectorAdd(XMLoadFloat3(&m_vMoveDir), XMLoadFloat3(&pMoveCommand->Get_Dir())));
            }

            return true;
        }
    }

    return false;
}

void CKirby_Ability_Sleep::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    __super::On_Damaged_KirbyState(pKirby, tInfo);
}

void CKirby_Ability_Sleep::Change_SleepState(CKirby* pKirby, SLEEP_STATE eNext)
{
    if (m_eSleepState == eNext)
        return;

    Exit_SleepState(pKirby, m_eSleepState);

    m_eSleepState = eNext;

    Enter_SleepState(pKirby, m_eSleepState);
}

void CKirby_Ability_Sleep::Enter_SleepState(CKirby* pKirby, SLEEP_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (eState)
    {
        case SLEEP_STATE::SLEEP_WAIT_START:
        {
            pAnimator->Play("SleepWaitStart", false, false, 0.1f, 1.5f);
            break;
        }
        case SLEEP_STATE::SLEEP_MOVE_ACTIVE:
        {
            if (!pKirby->Get_Movement()->Is_Grounded())
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_FALL);
            else if (Has_MoveDir())
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_WALK);
            else
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_WAIT);

            break;
        }
        case SLEEP_STATE::SLEEP_START:
        {
            pAnimator->Play("SleepStart", false, false, 0.1f, 1.5f);
            break;
        }
        case SLEEP_STATE::SLEEP:
        {
            pAnimator->Play("Sleep", false, true, 0.1f, 1.5f);
            --m_iSleepAniCount;
            break;
        }
        case SLEEP_STATE::SLEEP_END_WAKE_UP:
        {
            pAnimator->Play("SleepEndWakeUp", false, false, 0.1f, 1.5f);
            break;
        }
        case SLEEP_STATE::SLEEP_STATE_END:
        {
            m_bReqEndAttackState = true;
            break;
        }
    }
}

void CKirby_Ability_Sleep::Update_SleepState(CKirby* pKirby, _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eSleepState)
    {
        case SLEEP_STATE::SLEEP_WAIT_START:
        {
            if (SleepBeforeTimer(fTimeDelta))
            {
                Change_SleepState(pKirby, SLEEP_STATE::SLEEP_START);
                break;
            }

            _bool m_bIsGround = pKirby->Get_Movement()->Is_Grounded();
            if (pAnimator->Is_Finished() || m_bIsGround || Has_MoveDir())
                Change_SleepState(pKirby, SLEEP_STATE::SLEEP_MOVE_ACTIVE);  
            break;
        }
        case SLEEP_STATE::SLEEP_MOVE_ACTIVE:
        {
            if(SleepBeforeTimer(fTimeDelta))
            {
                Change_SleepState(pKirby, SLEEP_STATE::SLEEP_START);
                break;
            }

            Update_SleepBeforeState(pKirby, fTimeDelta);
            break;
        }
        case SLEEP_STATE::SLEEP_START:
        {
            if (pAnimator->Is_Finished())
                Change_SleepState(pKirby, SLEEP_STATE::SLEEP);
            break;
        }
        case SLEEP_STATE::SLEEP:
        {
            if (m_iSleepAniCount == 0)
            {
                Change_SleepState(pKirby, SLEEP_STATE::SLEEP_END_WAKE_UP);
                break;
            }

            if (pAnimator->Is_Finished())
            {
                pAnimator->Play("Sleep", false, true, 0.1f, 1.5f);
                --m_iSleepAniCount;
            }
            break;
        }
        case SLEEP_STATE::SLEEP_END_WAKE_UP:
        {
            if (pAnimator->Is_Finished())
                Change_SleepState(pKirby, SLEEP_STATE::SLEEP_STATE_END);
            break;
        }
    }
}

void CKirby_Ability_Sleep::Exit_SleepState(CKirby* pKirby, SLEEP_STATE eState)
{
    switch (eState)
    {
        case SLEEP_STATE::SLEEP_WAIT_START:
            break;
        case SLEEP_STATE::SLEEP_MOVE_ACTIVE:
        {
            Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_BEFORE_STATE_END);
            m_vMoveDir = {};
            break;
        }
        case SLEEP_STATE::SLEEP_START:
        case SLEEP_STATE::SLEEP:
        case SLEEP_STATE::SLEEP_END_WAKE_UP:
        case SLEEP_STATE::SLEEP_STATE_END:
            break;
    }
}

void CKirby_Ability_Sleep::Change_SleepBeforeState(CKirby* pKirby, SLEEP_BEFORE_STATE eNext)
{
    if (m_eSleepBeforeState == eNext)
        return;

    Exit_SleepBeforeState(pKirby, m_eSleepBeforeState);

    m_eSleepBeforeState = eNext;

    Enter_SleepBeforeState(pKirby, m_eSleepBeforeState);
}

void CKirby_Ability_Sleep::Enter_SleepBeforeState(CKirby* pKirby, SLEEP_BEFORE_STATE eState)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (eState)
    {
        case SLEEP_BEFORE_STATE::SLEEP_WAIT:
        {
            pAnimator->Play("SleepWait", true, false, 0.1f, 1.5f);
            break;
        }
        case SLEEP_BEFORE_STATE::SLEEP_WALK:
        {
            pAnimator->Play("Sleep_SleepWalk", true, false, 0.1f, 1.5f);
            break;
        }
        case SLEEP_BEFORE_STATE::SLEEP_FALL:
        {
            pAnimator->Play("SleepFall", true, false, 0.1f, 1.5f);
            break;
        }
        case SLEEP_BEFORE_STATE::SLEEP_BEFORE_STATE_END:
        {
            break;
        }
    }
}

void CKirby_Ability_Sleep::Update_SleepBeforeState(CKirby* pKirby, _float fTimeDelta)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    _bool m_bIsGround = pMovement->Is_Grounded();
    switch (m_eSleepBeforeState)
    {
        case SLEEP_BEFORE_STATE::SLEEP_WAIT:
        {
            if (!m_bIsGround)
            {
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_FALL);
            }
            else if (Has_MoveDir())
            {
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_WALK);
                pKirby->Add_MoveDir(m_vMoveDir);
                m_vMoveDir = {};
            }
            break;
        }
        case SLEEP_BEFORE_STATE::SLEEP_WALK:
        {
            if (!m_bIsGround)
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_FALL);
            else if (!Has_MoveDir())
                Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_WAIT);
            else
                pKirby->Add_MoveDir(m_vMoveDir);

            m_vMoveDir = {};
            break;
        }
        case SLEEP_BEFORE_STATE::SLEEP_FALL:
        {
            if (m_bIsGround)
            {
                if (Has_MoveDir())
                {
                    Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_WALK);
                    pKirby->Add_MoveDir(m_vMoveDir);
                    m_vMoveDir = {};
                }
                else
                {
                    Change_SleepBeforeState(pKirby, SLEEP_BEFORE_STATE::SLEEP_WAIT);
                }
            }
            else
            {
                pKirby->Add_MoveDir(m_vMoveDir);
                m_vMoveDir = {};
            }
            break;
        }
        case SLEEP_BEFORE_STATE::SLEEP_BEFORE_STATE_END:
            break;
    }
}

void CKirby_Ability_Sleep::Exit_SleepBeforeState(CKirby* pKirby, SLEEP_BEFORE_STATE eState)
{
    switch (eState)
    {
        case SLEEP_BEFORE_STATE::SLEEP_WAIT:
        case SLEEP_BEFORE_STATE::SLEEP_WALK:
        case SLEEP_BEFORE_STATE::SLEEP_FALL:
        case SLEEP_BEFORE_STATE::SLEEP_BEFORE_STATE_END:
            break;
    }
}

_bool CKirby_Ability_Sleep::Has_MoveDir()
{
    _vector vMoveDir = XMLoadFloat3(&m_vMoveDir);

    if (XMVector3Equal(vMoveDir, XMVectorZero()))
        return false;

    return true;
}

_bool CKirby_Ability_Sleep::SleepBeforeTimer(_float fTimeDelta)
{
    if (m_fAccSleepBeforeTime > 0.f) {
        m_fAccSleepBeforeTime -= fTimeDelta;

        if (m_fAccSleepBeforeTime <= 0.f)
        {
            m_fAccSleepBeforeTime = 0.f;
            return true;
        }
    }

    return false;
}

CKirby_Ability_Sleep* CKirby_Ability_Sleep::Create()
{
    CKirby_Ability_Sleep* pInstance = new CKirby_Ability_Sleep();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Ability_Sleep");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Ability_Sleep::Free()
{
    __super::Free();
}
