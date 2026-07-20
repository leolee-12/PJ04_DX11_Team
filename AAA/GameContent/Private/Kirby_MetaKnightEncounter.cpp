#include "Kirby_MetaKnightEncounter.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_MetaKnightEncounter::CKirby_MetaKnightEncounter()
{
}

HRESULT CKirby_MetaKnightEncounter::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_MetaKnightEncounter::Get_StateType()
{
    return KIRBY_STATE_TYPE::METAKNIGHT_ENCOUNTER;
}

void CKirby_MetaKnightEncounter::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    m_bIsPositioned = false;

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();
    pKirby->Get_KirbyAbility()->Clear_Overlay(pKirby);

    CAnimator::ANI_PLAY_INFO tInfo{};
    tInfo.bLoop = false;
    tInfo.bRestart = false;
    tInfo.fBlend = 0.1f;
    tInfo.fSpeed = 1.5f;

    tInfo.strAniName = "LookAround";
    pAnimator->Play(&tInfo);

    tInfo.strAniName = "Metaknight_DemoAppearCut1";
    pAnimator->Enqueue(tInfo);

    tInfo.strAniName = "Metaknight_DemoAppearCut2";
    pAnimator->Enqueue(tInfo);

    tInfo.strAniName = "Metaknight_DemoAppearCut3";
    pAnimator->Enqueue(tInfo);

    tInfo.strAniName = "Metaknight_DemoAppearCut4";
    pAnimator->Enqueue(tInfo);

    tInfo.strAniName = "Metaknight_DemoAppearCut5";
    pAnimator->Enqueue(tInfo);
}

void CKirby_MetaKnightEncounter::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CAnimator* pAnimator = pKirby->Get_Body()->Get_Animator();

    if (m_bIsPositioned == false && pAnimator->Get_CurrentAnimName() == "Metaknight_DemoAppearCut1")
    {
        if (m_matAnchorWorld._44 <= Helper::fEpsilon)
        {
            MSG_BOX("m_matAnchorWorld has not been set : CKirby_MetaKnightEncounter");
        }
        else
        {
            CTransform* pTransform = pKirby->Get_Transform();
            pTransform->Set_WorldMatrix(m_matAnchorWorld);
        }

        m_bIsPositioned = true;
    }

    if (pAnimator->Get_CurrentAnimName() == "Metaknight_DemoAppearCut5" && pAnimator->Is_Finished())
    {

        Transition_Fall_OR_Wait_OR_Run_Immediate(pKirby);
    }
}

void CKirby_MetaKnightEncounter::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

_bool CKirby_MetaKnightEncounter::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_MetaKnightEncounter::Request_PositionSync(CKirby* pKirby, const KIRBY_POSITION_SYNC_BEGIN_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_CONTEXT::METAKNIGHT_ENCOUNTER:
        {
            m_matAnchorWorld = pDesc->AnchorWorld;
            break;
        }
        default:
        {
            MSG_BOX("Event Error 1: CKirby_MetaKnightEncounter");
            break;
        }
    }
}

void CKirby_MetaKnightEncounter::Request_PositionSync_End(CKirby* pKirby, const KIRBY_POSITION_SYNC_END_DESC* pDesc)
{
    switch (pDesc->eType)
    {
        case KIRBY_POSITION_SYNC_END_REASON::METAKNIGHT_ENCOUNTER_END:
        {
            break;
        }
        default:
        {
            MSG_BOX("Event Error 2: CKirby_MetaKnightEncounter");
            break;
        }
    }
}

void CKirby_MetaKnightEncounter::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

CKirby_MetaKnightEncounter* CKirby_MetaKnightEncounter::Create()
{
    CKirby_MetaKnightEncounter* pInstance = new CKirby_MetaKnightEncounter();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_MetaKnightEncounter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_MetaKnightEncounter::Free()
{
    __super::Free();
}