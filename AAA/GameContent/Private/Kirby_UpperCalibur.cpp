#include "Kirby_UpperCalibur.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_UpperCalibur::CKirby_UpperCalibur()
{
}

HRESULT CKirby_UpperCalibur::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_UpperCalibur::Get_StateType()
{
    return KIRBY_STATE_TYPE::METAKNIGHT_UPPERCALIBUR;
}

void CKirby_UpperCalibur::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);
}

void CKirby_UpperCalibur::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    //if (pAnimator->Get_CurrentAnimName() == "Metaknight_DemoUpperCaliburCut7" && pAnimator->Is_Finished())
    //{
    //    Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
    //}
}

void CKirby_UpperCalibur::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_UpperCalibur::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_UpperCalibur::Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_UPPERCALIBUR:
        {
            // À§Ä¡
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

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut1";
            pAnimator->Play(&tInfo);

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut2";
            pAnimator->Enqueue(tInfo);

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut3";
            pAnimator->Enqueue(tInfo);

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut4";
            pAnimator->Enqueue(tInfo);

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut5";
            pAnimator->Enqueue(tInfo);

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut6";
            pAnimator->Enqueue(tInfo);

            tInfo.strAniName = "Metaknight_DemoUpperCaliburCut7";
            pAnimator->Enqueue(tInfo);
            break;
        }
        default:
        {
            MSG_BOX("Event Error 1: CKirby_UpperCalibur");
            break;
        }
    }
}

void CKirby_UpperCalibur::Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_UPPERCALIBUR_END:
        {
            // Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
            break;
        }    
        default:
        {
            MSG_BOX("Event Error 2: CKirby_UpperCalibur");
            break;
        }
    }
}

void CKirby_UpperCalibur::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

CKirby_UpperCalibur* CKirby_UpperCalibur::Create()
{
    CKirby_UpperCalibur* pInstance = new CKirby_UpperCalibur();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_UpperCalibur");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_UpperCalibur::Free()
{
    __super::Free();
}