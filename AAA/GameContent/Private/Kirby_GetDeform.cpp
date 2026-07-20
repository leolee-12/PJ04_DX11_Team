#include "Kirby_GetDeform.h"

#include "GameInstance.h"
#include "Effect_Loader.h"

#include "InhaleContainer.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

#include "Movement_Child.h"

#include "Deformable.h"

CKirby_GetDeform::CKirby_GetDeform()
{
}

HRESULT CKirby_GetDeform::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_GetDeform::Get_StateType()
{
    return KIRBY_STATE_TYPE::GET_DEFORM;
}

void CKirby_GetDeform::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    Subscribe_DeformEvent(pKirby);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GravityScale(0.f);
    pMovement->Set_Velocity(XMVectorSet(0.f, 0.f, 0.f, 0.f));
    pMovement->Clear_Forces();

    pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);
      
    IDeformable* pDeformable = pKirby->Get_TriggerDeformObj();
    if (pDeformable == nullptr)
    {
        assert(false);
        return;
    }

    Capture_DeformContext(pDeformable);
    pPendingDeform = pKirby->Find_KirbyDeform(m_tPostDeformEndContext.eDeformType);

    pDeformable->Request_Deform(pKirby->Get_Transform()->Get_WorldMatrixPtr());

    m_eDeformState = DEFORM_STATE::DEFORM_STATE_END;
    Change_GetDeformState(pKirby, DEFORM_STATE::SUPER_INHALE_START);
}

void CKirby_GetDeform::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_GetDeformState(pKirby, fTimeDelta);
}

void CKirby_GetDeform::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);

    Unsubscribe_DeformEvent();

    pPendingDeform = nullptr;
}

_bool CKirby_GetDeform::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_GetDeform::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

void CKirby_GetDeform::Change_GetDeformState(CKirby* pKirby, DEFORM_STATE eNext)
{
    if (m_eDeformState == eNext)
        return;

    Exit_GetDeformState(pKirby, m_eDeformState);

    m_eDeformState = eNext;

    Enter_GetDeformState(pKirby, m_eDeformState);
}

void CKirby_GetDeform::Enter_GetDeformState(CKirby* pKirby, DEFORM_STATE eState)
{
    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pBodyAnimator = pBody->Get_Animator();

    switch (m_eDeformState)
    {
        case DEFORM_STATE::SUPER_INHALE_START:
        {
            CEffect_Loader::GetInstance()->Spawn(L"InhaleContainer", pKirby->Get_LevelIndex(),
                _float3(0.f, 0.93f, 0.4f), _float3(0.f, 0.f, 1.f), _float3(0.f, 0.f, 0.f),
                pKirby->Get_Transform()->Get_WorldMatrixPtr(), &m_pInhaleEffect);
            static_cast<CInhaleContainer*>(m_pInhaleEffect)->On_SuperInhale();

            pBodyAnimator->Play("SuperInhaleStart", false, false, 0.1f, 2.5f);

            pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), false, true);

            break;
        }
        case DEFORM_STATE::SUPER_INHALE_LOOP:
        {
            pBodyAnimator->Play("SuperInhale", true, false, 0.05f, 1.5f);
            break;
        }
        case DEFORM_STATE::DEFORM_STATE_DEFORM:
        {
            m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_DeformingSwallow1.wav", 0.2f);

            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(m_tPostDeformEndContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            pDeformModel_Demo->Set_Active(true);
            pDeformModel_Demo->Get_Animator()->Play("Deform", false, true, 0.1f, 1.8f);

            pKirby->Change_HatSocketMatrix(pKirby->Get_KirbyAbility()->Get_AbilityType(),
                pDeformModel_Demo->Get_HatBoneMatirx());

            pPendingDeform->Enter_DeformState_Deform(pKirby, m_tPostDeformEndContext);
            
            break;
        }
        case DEFORM_STATE::DEFORM_STATE_DEFORM_END:
        {
            pPendingDeform->Enter_DeformState_Deform_End(pKirby, m_tPostDeformEndContext);

            // Sound
            m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_GetAbility.wav", 0.2f);

            // 모자 교체
            COPY_ABILITY_TYPE eAbilityType = pKirby->Get_KirbyAbility()->Get_AbilityType();
            CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(m_tPostDeformEndContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
            pKirby->Change_HatSocketMatrix(eAbilityType, pDeformModel_Main->Get_HatBoneMatirx());

            // 이름 바꾸기
            KIRBY_NAME_UPDATED tNameDesc{};
            tNameDesc.strAtkModeName = pPendingDeform->Get_AttackModeName();
            m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tNameDesc);

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_END:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_GravityScale(1.f);

            pKirby->Change_KirbyDeform(m_tPostDeformEndContext.eDeformType);

            if (pPendingDeform->Should_ForceEnterAttackState())
                pKirby->Change_State(KIRBY_STATE_TYPE::ATTACK);
            else
                Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
            break;
        }
    }
}

void CKirby_GetDeform::Update_GetDeformState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eDeformState)
    {
        case DEFORM_STATE::SUPER_INHALE_START:
        {
            CKirby_Body* pBody = pKirby->Get_Body();
            CAnimator* pBodyAnimator = pBody->Get_Animator();

            if (pBodyAnimator->Is_Finished())
                Change_GetDeformState(pKirby, DEFORM_STATE::SUPER_INHALE_LOOP);

            break;
        }
        case DEFORM_STATE::SUPER_INHALE_LOOP:
        {
            if (m_tPostDeformEndContext.eDeformKind == DEFORM_OBJECT_KIND::FIXED)
            {
                // Test
                CTransform* pTransform = pKirby->Get_Transform();
                CMovement_Child* pMovement = pKirby->Get_Movement();

                const _vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_tPostDeformEndContext.vStartPos), 1.f);
                const _vector vCurrentPos = pTransform->Get_State(STATE::POSITION);
                const _vector vToTarget = vTargetPos - vCurrentPos;

                constexpr _float fMoveSpeed = 15.f;
                const _float fMoveDistance = fMoveSpeed * fTimeDelta;
                const _float fDistanceSq = XMVectorGetX(XMVector3LengthSq(vToTarget));

                pMovement->Set_GravityScale(0.f);
                pMovement->Clear_Forces();

                if (fDistanceSq <= fMoveDistance * fMoveDistance)
                {
                    pTransform->Set_State(STATE::POSITION, vTargetPos);
                    pMovement->Set_Velocity(XMVectorZero());
                    pMovement->Sync_To_Controller();
                }
                else
                {
                    const _vector vMoveDirection = XMVector3Normalize(vToTarget);
                    pMovement->Set_Velocity(vMoveDirection * fMoveSpeed);
                }
            }

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_DEFORM:
        {
            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(m_tPostDeformEndContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            CAnimator* pDemoAnimator = pDeformModel_Demo->Get_Animator();

            _bool bAniFinished = pDemoAnimator->Is_Finished();
            _bool bDeformFinished = pPendingDeform->Update_DeformState_Deform(pKirby, m_tPostDeformEndContext, fTimeDelta);

            if(bAniFinished && bDeformFinished)
                Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_STATE_DEFORM_END);

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_DEFORM_END:
        {
            if (pPendingDeform->Update_DeformState_Deform_End(pKirby, m_tPostDeformEndContext, fTimeDelta))
                Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_STATE_END);

            break;
        }
    }
}

void CKirby_GetDeform::Exit_GetDeformState(CKirby* pKirby, DEFORM_STATE eState)
{
    switch (m_eDeformState)
        {
        case DEFORM_STATE::SUPER_INHALE_START:
        {
            break;
        }
        case DEFORM_STATE::SUPER_INHALE_LOOP:
        {
            break;
        }
        case DEFORM_STATE::DEFORM_STATE_DEFORM:
        {
            pPendingDeform->Exit_DeformState_Deform(pKirby, m_tPostDeformEndContext);
            break;
        }
        case DEFORM_STATE::DEFORM_STATE_DEFORM_END:
        {
            pPendingDeform->Exit_DeformState_Deform_End(pKirby, m_tPostDeformEndContext);

            // 카메라 줌
            KIRBY_ABILITY_CHANGED Desc{};
            Desc.bBegin = false;
            m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &Desc);

            m_pGameInstance_Proxy->Set_TimeScale(1.f);

            break;
        }
    }
}

void CKirby_GetDeform::Subscribe_DeformEvent(CKirby* pKirby)
{
    if (m_bSubscribedDeformEvent == true)
        return;

    m_DeformEvent = m_pGameInstance_Proxy->Subscribe(
        EventTag::Deform_Acquired,
        [this, pKirby](void* pData)
        {
            const auto* pDesc = static_cast<DEFORM_ACQUIRED_EVENT*>(pData);            
            Handle_DeformEvent(pKirby, pDesc);
        }
    );

    m_bSubscribedDeformEvent = true;
}

void CKirby_GetDeform::Unsubscribe_DeformEvent()
{
    if (m_bSubscribedDeformEvent == false)
        return;

    m_pGameInstance_Proxy->UnSubscribe(m_DeformEvent);

    m_bSubscribedDeformEvent = false;
}

void CKirby_GetDeform::Handle_DeformEvent(CKirby* pKirby, const DEFORM_ACQUIRED_EVENT* pDesc)
{
    Effect_Stop(m_pInhaleEffect);
    pKirby->Get_Body()->Stop_SoundHandle();

    m_pGameInstance_Proxy->Set_TimeScale(0.f);

    pKirby->Get_Body()->Set_Active(false);

    // 카메라 줌
    KIRBY_ABILITY_CHANGED tDesc{};
    tDesc.bBegin = true;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &tDesc);

    if(pPendingDeform->HasDemoModel())
        Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_STATE_DEFORM);
    else
        Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_STATE_DEFORM_END);
}

void CKirby_GetDeform::Capture_DeformContext(IDeformable* pDeformable)
{
    m_tPostDeformEndContext = {};
    m_tPostDeformEndContext.pDeformable = pDeformable;
    m_tPostDeformEndContext.eDeformType = pDeformable->Get_DeformType();
    m_tPostDeformEndContext.eDeformKind = pDeformable->Get_DeformKind();

    CGameObject* pDeformObject = pDeformable->Get_GameObject();
    CTransform* pTransform = pDeformObject->Get_Transform();
    const _matrix matDeformStartWorld = XMLoadFloat4x4(pTransform->Get_WorldMatrixPtr());

    XMStoreFloat3(&m_tPostDeformEndContext.vStartPos, matDeformStartWorld.r[3]);

    const _vector vStartLook = XMVector3Normalize(XMVectorSetY(matDeformStartWorld.r[2], 0.f));
    XMStoreFloat3(&m_tPostDeformEndContext.vStartLook, vStartLook);
}

CKirby_GetDeform* CKirby_GetDeform::Create()
{
    CKirby_GetDeform* pInstance = new CKirby_GetDeform();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_GetDeform");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_GetDeform::Free()
{
    Unsubscribe_DeformEvent();

    __super::Free();
}