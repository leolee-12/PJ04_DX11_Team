#include "Kirby_GetDeform.h"

#include "GameInstance.h"
#include "Effect_Loader.h"

#include "InhaleContainer.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Kirby_Deform.h"

#include "Movement_Child.h"

CKirby_GetDeform::CKirby_GetDeform()
{
}

HRESULT CKirby_GetDeform::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_GetDeform::Get_StateType()
{
    return KIRBY_STATE_TYPE::GET_DEFORM;
}

void CKirby_GetDeform::Enter(CKirby* pKirby)
{
    __super::Enter(pKirby);

    pKirby->Set_KirbyDeform(DEFORM_TYPE::CAR);

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
}

_bool CKirby_GetDeform::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    KIRBY_COMMAND_TYPE eCommandType = pCommand->GetCommandType();

    //switch (eCommandType)
    //{
    //    default:        
    //        break;
    //}

    return false;
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

            pKirby->OnOffParts(pKirby->Get_KirbyAbility()->Get_AbilityType(), false, true);

            break;
        }
        case DEFORM_STATE::SUPER_INHALE_LOOP:
        {
            pBodyAnimator->Play("SuperInhale", true, false, 0.05f, 1.5f);

            break;
        }
        case DEFORM_STATE::DEFORM:
        {
            pKirby->Get_Body()->Set_Active(false);

            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(DEFORM_TYPE::CAR, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            pDeformModel_Demo->Set_Active(true);
            pDeformModel_Demo->Get_Animator()->Play("Deform", false, false, 0.1f, 1.5f, true);

            break;
        }
        case DEFORM_STATE::DEFORM_END:
        {
            DEFORM_TYPE eDeformType = pKirby->Get_KirbyDeform()->Get_DeformType();
            CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
            pDeformModel_Main->Set_Active(true);

            pDeformModel_Main->Get_Animator()->Play("DemoEndFirst", false, false, 0.f, 2.f);

            break;
        }
        case DEFORM_STATE::DEFORM_STATE_END:
        {
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
            DEFORM_TYPE eDeformType = pKirby->Get_KirbyDeform()->Get_DeformType();
            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            CAnimator* pAnimator = pDeformModel_Demo->Get_Animator();

            if (pAnimator->Is_Finished())
                Change_GetDeformState(pKirby, DEFORM_STATE::DEFORM_END);

            break;
        }
        case DEFORM_STATE::DEFORM_END:
        {
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
            if (m_pInhaleEffect)
            {
                m_pInhaleEffect->EffectContainer_Stop();
                m_pInhaleEffect = nullptr;
            }
            break;
        }
        case DEFORM_STATE::DEFORM:
        {
            DEFORM_TYPE eDeformType = pKirby->Get_KirbyDeform()->Get_DeformType();
            CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
            pDeformModel_Demo->Set_Active(false);
            break;
        }
        case DEFORM_STATE::DEFORM_END:
        {
            break;
        }
        case DEFORM_STATE::DEFORM_STATE_END:
        {
            break;
        }
    }
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
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}