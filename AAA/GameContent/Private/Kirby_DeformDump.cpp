#include "Kirby_DeformDump.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Deformable.h"
#include "Kirby_Deform.h"

#include "Movement_CHild.h"

CKirby_DeformDump::CKirby_DeformDump()
{
}

HRESULT CKirby_DeformDump::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_DeformDump::Get_StateType()
{
    return KIRBY_STATE_TYPE::DEFORM_DUMP;
}

void CKirby_DeformDump::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    m_eDeformDumpState = DEFORM_DUMP_STATE_END;
    Change_DeformDumpState(pKirby, DEFORM_DUMP_STATE::SPIT_START);
}

void CKirby_DeformDump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    Update_DeformDumpState(pKirby, fTimeDelta);
}

void CKirby_DeformDump::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_DeformDump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_DeformDump::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

void CKirby_DeformDump::Change_DeformDumpState(CKirby* pKirby, DEFORM_DUMP_STATE eNext)
{
    if (m_eDeformDumpState == eNext)
        return;

    Exit_DeformDumpState(pKirby, m_eDeformDumpState);

    m_eDeformDumpState = eNext;

    Enter_DeformDumpState(pKirby, m_eDeformDumpState);
}

void CKirby_DeformDump::Enter_DeformDumpState(CKirby* pKirby, DEFORM_DUMP_STATE eState)
{
    switch (eState)
    {
        case DEFORM_DUMP_STATE::SPIT_START:
        {
            CKirby_Deform* pKirbyDeform = pKirby->Get_KirbyDeform();
            pKirbyDeform->Play_DeformAni(pKirby, DEFORM_ANI::SPIT_START);
            pKirby->Get_Movement()->Add_Velocity(XMVectorSet(0.f, 22.f, 0.f, 0.f));
            break;
        }
        case DEFORM_DUMP_STATE::SPIT_DEFORM:
        {
            // Deform Model
            CKirby_Deform* pKirbyDeform = pKirby->Get_KirbyDeform();
            DEFORM_TYPE m_eDeformType = pKirbyDeform->Get_DeformType();

            CKirby_Deform_Model* pDeformModel = pKirby->Get_DeformPart_Model(m_eDeformType);
            pDeformModel->Set_Active(false);

            // Body
            CKirby_Body* pBody = pKirby->Get_Body();
            pBody->Set_Active(true);
            CAnimator* pBodyAnimator = pBody->Get_Animator();
            pBodyAnimator->Play("SpitDeform", false, false, 0.1f, 2.f);

            // Parts
            COPY_ABILITY_TYPE m_eAbilityType = pKirby->Get_KirbyAbility()->Get_AbilityType();
            pKirby->Change_HatSocketMatrix(m_eAbilityType, pBody->Get_HatBoneMatirx());
            pKirby->Set_AbilityPartsActive(m_eAbilityType, true);

            // ptr
            pKirby->Get_HeldDeformObj()->End_Deform(pKirby->Get_Transform()->Get_WorldMatrixPtr());
            pKirby->Set_HeldDeformObj(nullptr);

            pKirby->Reset_KirbyDeform();

            // Name
            KIRBY_NAME_UPDATED tNameDesc{};
            tNameDesc.strAtkModeName = pKirby->Get_ActiveAttackMode()->Get_AttackModeName();
            m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tNameDesc);

            break;
        }
        case DEFORM_DUMP_STATE::DEFORM_DUMP_STATE_END:
        {
            Transition_Fall_OR_Wait_OR_Run(pKirby);
            break;
        }
    }
}

void CKirby_DeformDump::Update_DeformDumpState(CKirby* pKirby, _float fTimeDelta)
{
    switch (m_eDeformDumpState)
    {
        case DEFORM_DUMP_STATE::SPIT_START:
        {
            CKirby_Deform* pKirbyDeform = pKirby->Get_KirbyDeform();
            DEFORM_TYPE m_eDeformType = pKirbyDeform->Get_DeformType();

            CKirby_Deform_Model* pDeformModel = pKirby->Get_DeformPart_Model(m_eDeformType);
            CAnimator* pDeformModelAnimator = pDeformModel->Get_Animator();

            if (pDeformModelAnimator->Is_Finished())
                Change_DeformDumpState(pKirby, DEFORM_DUMP_STATE::SPIT_DEFORM);

            break;
        }
        case DEFORM_DUMP_STATE::SPIT_DEFORM:
        {
            CKirby_Body* pBody = pKirby->Get_Body();
            CAnimator* pBodyAnimator = pBody->Get_Animator();

            if (pBodyAnimator->Is_Finished())
                Change_DeformDumpState(pKirby, DEFORM_DUMP_STATE::DEFORM_DUMP_STATE_END);
            break;
        }
    }
}

void CKirby_DeformDump::Exit_DeformDumpState(CKirby* pKirby, DEFORM_DUMP_STATE eState)
{
    switch (eState)
    {
        case DEFORM_DUMP_STATE::SPIT_START:

            break;
        case DEFORM_DUMP_STATE::SPIT_DEFORM:

            break;
    }
}

CKirby_DeformDump* CKirby_DeformDump::Create()
{
    CKirby_DeformDump* pInstance = new CKirby_DeformDump();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformDump");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformDump::Free()
{
    __super::Free();
}
