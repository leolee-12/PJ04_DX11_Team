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
    
    pKirby->Set_HeldDeformObj(pKirby->Get_TriggerDeformObj());    
    IDeformable* pDeformable = pKirby->Get_HeldDeformObj();
    m_eDeformType = pDeformable->Get_DeformType();

    // 
    pDeformable->Request_Deform();
    pDeformable->Begin_Deform(pKirby->Get_Transform()->Get_WorldMatrixPtr());

    m_eDeformState = DEFORM_STATE::DEFORM_STATE_END;
    Change_GetDeformState(pKirby, DEFORM_STATE::SUPER_INHALE_START);

    pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);
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
        case DEFORM_STATE::DEFORM:
        {
            if (m_pInhaleEffect)
            {
                m_pInhaleEffect->EffectContainer_Stop();
                m_pInhaleEffect = nullptr;
            }
            pKirby->Get_Body()->Stop_SoundHandle();

            KIRBY_ABILITY_CHANGED tDesc{};
            tDesc.bBegin = true;
            m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &tDesc);
            m_pGameInstance_Proxy->Set_TimeScale(0.f);

            pKirby->Get_Body()->Set_Active(false);

            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(m_eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            pDeformModel_Demo->Set_Active(true);
            pDeformModel_Demo->Get_Animator()->Play("Deform", false, false, 0.1f, 1.8f);

            pKirby->Change_HatSocketMatrix(pKirby->Get_KirbyAbility()->Get_AbilityType(),
                pDeformModel_Demo->Get_HatBoneMatirx());

            break;
        }
        case DEFORM_STATE::DEFORM_END:
        {
            CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(m_eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
            pDeformModel_Main->Set_Active(true);

            pDeformModel_Main->Get_Animator()->Play("DemoEndFirst", false, false, 0.f, 3.5f);

            COPY_ABILITY_TYPE m_eAbilityType = pKirby->Get_KirbyAbility()->Get_AbilityType();
            pKirby->Change_HatSocketMatrix(m_eAbilityType, pDeformModel_Main->Get_HatBoneMatirx());

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_RotationSpeed(s_fDeform_End_Rot_Speed_Degree);
            Set_RotationDir(pKirby);
            
            pKirby->Change_KirbyDeform(m_eDeformType);

            // 이름 바꾸기
            KIRBY_NAME_UPDATED tNameDesc{};
            tNameDesc.strAtkModeName = pKirby->Get_ActiveAttackMode()->Get_AttackModeName();
            m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tNameDesc);

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_END:
        {
            pKirby->Change_State(KIRBY_STATE_TYPE::WAIT);
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
            // 충돌 판정나면
            if (GetAsyncKeyState('F') & 0x8000)
            {
                Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM);
            }
            break;
        }
        case DEFORM_STATE::DEFORM:
        {
            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(m_eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            CAnimator* pDemoAnimator = pDeformModel_Demo->Get_Animator();

            if (pDemoAnimator->Is_Finished())
                Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_END);

            break;
        }
        case DEFORM_STATE::DEFORM_END:
        {
            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Rotate_To_Direction(XMLoadFloat3(&m_vRotationDir), fTimeDelta);

            CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(m_eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
            CAnimator* pMainAnimator = pDeformModel_Main->Get_Animator();

            if (pMainAnimator->Is_Finished())
                Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_STATE_END);

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_END:
        {
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
        case DEFORM_STATE::DEFORM:
        {
            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(m_eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            pDeformModel_Demo->Set_Active(false);
            break;
        }
        case DEFORM_STATE::DEFORM_END:
        {
            KIRBY_ABILITY_CHANGED Desc{};
            Desc.bBegin = false;
            m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &Desc);
            m_pGameInstance_Proxy->Set_TimeScale(1.f);

            CMovement_Child* pMovement = pKirby->Get_Movement();
            pMovement->Set_RotationSpeed(CKirby::s_fRot_Speed_Degree);

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_END:
        {
            break;
        }
    }
}

void CKirby_GetDeform::Set_RotationDir(CKirby* pKirby)
{
    _vector vCamPos = XMLoadFloat4(m_pGameInstance_Proxy->Get_CamPosition());
    _vector vPlayerPos = pKirby->Get_Transform()->Get_State(STATE::POSITION);
    _vector vDir = XMVectorSetY(vCamPos - vPlayerPos, 0.f);
    vDir = XMVector3Normalize(vDir);

    _float fRadian = XMConvertToRadians(15.f);

    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vLeftDir = XMVector3Rotate(vDir, XMQuaternionRotationAxis(vUp, -fRadian));
    _vector vRightDir = XMVector3Rotate(vDir, XMQuaternionRotationAxis(vUp, fRadian));

    _vector vCurLook = XMVectorSetY(pKirby->Get_Transform()->Get_State(STATE::LOOK), 0.f);
    vCurLook = XMVector3Normalize(vCurLook);

    _float fLeftDot = XMVectorGetX(XMVector3Dot(vCurLook, vLeftDir));
    _float fRightDot = XMVectorGetX(XMVector3Dot(vCurLook, vRightDir));

    if (fLeftDot > fRightDot)
         XMStoreFloat3(&m_vRotationDir, vLeftDir);
    else
        XMStoreFloat3(&m_vRotationDir, vRightDir);
}

void CKirby_GetDeform::Subscribe_DeformEvent(CKirby* pKirby)
{
    if (m_bSubscribedDeformEvent == true)
        return;

    m_DeformEvent = m_pGameInstance_Proxy->Subscribe(
        EventTag::Deform_Acquired,
        [this, pKirby](void* pData)
        {
            Handle_DeformEvent(pKirby);
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

void CKirby_GetDeform::Handle_DeformEvent(CKirby* pKirby)
{
    Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM);
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