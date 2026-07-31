#include "Kirby_Slide.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"
#include "Kirby_Jump.h"

#include "Movement_Child.h"

#include "Monster.h"

CKirby_Slide::CKirby_Slide()
{
}

HRESULT CKirby_Slide::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_Slide::Get_StateType()
{
    return KIRBY_STATE_TYPE::SLIDE;
}

void CKirby_Slide::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    m_eSlideState = SLIDE_STATE::STATE_END;
    Change_SlideState(pKirby, SLIDE_STATE::SLIDE_START);
}

void CKirby_Slide::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_SlideState(pKirby, fTimeDelta);
}

void CKirby_Slide::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GroundFriction(CKirby::s_fGroundFriction);
    pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);
}

_bool CKirby_Slide::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    switch (eCommandType)
    {
        // Jump Down
        case KIRBY_COMMAND_TYPE::JUMP:
        {
            if (!pCommand->IsDown())
                return false;

            if (m_eSlideState == SLIDE_STATE::SLIDE &&
                (m_fSlideTime / 2.f) > 0.3f)
                return true;

            pKirby->Change_State(KIRBY_STATE_TYPE::JUMP, JUMP_STATE_FLAG::JUMP_FROM_SLIDE);
            return true;
        }
    }

    return false;
}

void CKirby_Slide::On_KirbyCollisionEnter(CKirby* pKirby, _uint iColliderType, CCollider* pOther)
{
    switch (iColliderType)
    {
        case CKirby::KIRBY_COLLIDER::SLIDE_COLLIDER:
        {
            const _uint iGroup = pOther->Get_RegisteredGroup();
            CGameObject* pGameObject = pOther->Get_Owner();
            if (iGroup == ETOUI(COLLISION_LAYER::MONSTER_HURT))
            {
                CMonster* pMonster = dynamic_cast<CMonster*>(pGameObject);
                if (pMonster == nullptr)
                    return;

                if (!pMonster->Is_Touch_Harmful())
                    return;

                ATTACK_INFO tAttackDesc{};
                tAttackDesc.eHitType = HIT_TYPE::SLIDE;
                tAttackDesc.pAttacker = pKirby;
                XMStoreFloat3(&tAttackDesc.vAttackerPos, pKirby->Get_Transform()->Get_State(STATE::POSITION));
                tAttackDesc.fDamage = 8.f;
                tAttackDesc.fKnockback = 10.f;
                pMonster->Damaged(tAttackDesc);

                Change_SlideState(pKirby, SLIDE_STATE::BACK_JUMP);
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::ENV_HURT) || iGroup == ETOUI(COLLISION_LAYER::NPC_HURT))
            {
                if (auto* pDamageable = dynamic_cast<IDamageable*>(pGameObject))
                {
                    ATTACK_INFO tAttackDesc{};
                    tAttackDesc.eHitType = HIT_TYPE::SLIDE;
                    tAttackDesc.pAttacker = pKirby;
                    XMStoreFloat3(&tAttackDesc.vAttackerPos, pKirby->Get_Transform()->Get_State(STATE::POSITION));
                    tAttackDesc.fDamage = 8.f;
                    tAttackDesc.fKnockback = 10.f;
                    pDamageable->Damaged(tAttackDesc);
                }

                if (iGroup == ETOUI(COLLISION_LAYER::NPC_HURT))
                    Change_SlideState(pKirby, SLIDE_STATE::BACK_JUMP);
            }
            break;
        }
    }
}

void CKirby_Slide::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

void CKirby_Slide::Change_SlideState(CKirby* pKirby, SLIDE_STATE eNextState)
{
    if (m_eSlideState == eNextState)
        return;

    Exit_SlideState(pKirby, m_eSlideState);

    m_eSlideState = eNextState;

    Enter_SlideState(pKirby, m_eSlideState);
}

void CKirby_Slide::Enter_SlideState(CKirby* pKirby, SLIDE_STATE eState)
{
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();

    switch (eState)
    {
        case SLIDE_STATE::SLIDE_START:
            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::SLIDE_START);
            break;

        case SLIDE_STATE::SLIDE:
        {
            m_fSlideTime = s_fMaxSlideTime;

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_GroundFriction(s_fSlideGroundFriction);
            pMovement->Set_MaxHorizontalSpeed(s_fMaxSlideHorizontalSpeed);

            XMStoreFloat3(&m_vSlideDir, XMVector3Normalize(
                XMVectorSetY(pKirby->Get_Transform()->Get_State(STATE::LOOK), 0.f)));

            constexpr _float fMax = 999.f;
            pMovement->Add_Velocity(XMLoadFloat3(&m_vSlideDir) * fMax);

            pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::SLIDE);
            break;
        }
        case SLIDE_STATE::BACK_JUMP:
        {
            pKirby->Get_Body()->Get_Animator()->Play("BackJump", false, false, 0.1f, 2.f);
            CMovement_Child* pMovement = pKirby->Get_Movement();

            _vector vBackDir = -pKirby->Get_Transform()->Get_State(STATE::LOOK);
            vBackDir = XMVector3Normalize(XMVectorSetY(vBackDir, 0.f));
            pMovement->Set_Velocity(vBackDir * 10.f);

            pMovement->Force_Jump(12.f);  
            break;
        }
        case SLIDE_STATE::STATE_END:
            Transition_Fall_OR_Wait_OR_Run(pKirby);
            break;
    }
}

void CKirby_Slide::Update_SlideState(CKirby* pKirby, const _float fTimeDelta)
{
    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    switch (m_eSlideState)
    {
        case SLIDE_STATE::SLIDE_START:
            if (pAnimator->Is_Finished())
                Change_SlideState(pKirby, SLIDE_STATE::SLIDE);
            break;

        case SLIDE_STATE::SLIDE:
        {
            m_fSlideTime -= fTimeDelta;

            _float fRatio = 1.f - (m_fSlideTime / s_fMaxSlideTime);
            Helper::FloatClamp(fRatio, 0.f, 1.f);
            pAnimator->Seek(fRatio);

            if (m_fSlideTime <= 0.f)
            {
                Change_SlideState(pKirby, SLIDE_STATE::STATE_END);
                m_fSlideTime = 0.f;
            }

            break;
        }

        case SLIDE_STATE::BACK_JUMP:
            if (pAnimator->Is_Finished())
                Change_SlideState(pKirby, SLIDE_STATE::STATE_END);
            break;

        case SLIDE_STATE::STATE_END:
            break;
    }
}

void CKirby_Slide::Exit_SlideState(CKirby* pKirby, SLIDE_STATE eState)
{
    switch (eState)
    {
        case SLIDE_STATE::SLIDE_START:
            break;

        case SLIDE_STATE::SLIDE:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_GroundFriction(CKirby::s_fGroundFriction);
            pMovement->Set_MaxHorizontalSpeed(CKirby::s_fMaxHorizontalSpeed);

            pKirby->Get_Body()->Set_KirbyEye(KIRBY_EYE_STATE::IDLE);
            break;
        }
        case SLIDE_STATE::BACK_JUMP:
            break;

        case SLIDE_STATE::STATE_END:
            break;
    }
}

CKirby_Slide* CKirby_Slide::Create()
{
    CKirby_Slide* pInstance = new CKirby_Slide();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_Slide");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Slide::Free()
{
    __super::Free();
}