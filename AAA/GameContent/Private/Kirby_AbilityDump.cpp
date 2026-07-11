#include "Kirby_AbilityDump.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_Ability.h"

#include "Bubble_Manager.h"

CKirby_AbilityDump::CKirby_AbilityDump()
{
}

HRESULT CKirby_AbilityDump::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_pBubbleManager = CBubble_Manager::GetInstance();

    return S_OK;
}

KIRBY_STATE_TYPE CKirby_AbilityDump::Get_StateType()
{
    return KIRBY_STATE_TYPE::ABILITY_DUMP;
}

void CKirby_AbilityDump::Enter(CKirby* pKirby, _int iFlag)
{
    __super::Enter(pKirby, iFlag);

    // Ani
    CKirby_Ability* pAbility = pKirby->Get_KirbyAbility();
    pAbility->Clear_Overlay(pKirby, 1, 0.1f);
    pAbility->Play_AbilityAni(pKirby, ABILITY_ANI::ABILITY_DUMP);

    m_bPartsOff = false;
}

void CKirby_AbilityDump::Update(CKirby* pKirby, const _float fTimeDelta)
{
    __super::Update(pKirby, fTimeDelta);

    CKirby_Body* pBody = pKirby->Get_Body();
    CAnimator* pAnimator = pBody->Get_Animator();

    const _float fRatio = pAnimator->Get_Progress();

    Update_AbilityDump(pKirby, fRatio);

    if(pAnimator->Is_Finished())
    {
        Transition_Fall_OR_Wait_OR_Run(pKirby);
        return;
    }
}

void CKirby_AbilityDump::Exit(CKirby* pKirby)
{
    __super::Exit(pKirby);
}

void CKirby_AbilityDump::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
}

_bool CKirby_AbilityDump::Handle_Command(CKirby* pKirby, CKirby_Command* pCommand)
{
    if (__super::Handle_Command(pKirby, pCommand))
        return true;

    return false;
}

void CKirby_AbilityDump::Update_AbilityDump(CKirby* pKirby, _float fRatio)
{
    if (m_bPartsOff == false && fRatio >= 0.45f)
    {
        Spawn_DroppedBubble(pKirby);

        pKirby->Set_AbilityPartsActive(pKirby->Get_KirbyAbility()->Get_AbilityType(), false);
        pKirby->Request_ChangeKirbyAbility(COPY_ABILITY_TYPE::NORMAL);
        pKirby->Apply_ChangeKirbyAbility();

        KIRBY_NAME_UPDATED tNameDesc{};
        tNameDesc.strAtkModeName = pKirby->Get_KirbyAbility()->Get_AttackModeName();
        m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tNameDesc);

        m_bPartsOff = true;
    }
}

void CKirby_AbilityDump::Spawn_DroppedBubble(CKirby* pKirby)
{
    CTransform* pTransform = pKirby->Get_Transform();
    _vector vSpawnPos = pTransform->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vDir = -pTransform->Get_State(STATE::LOOK);

    m_pBubbleManager->Spawn(pKirby->Get_LevelIndex(), CBubble_Manager::BUBBLE_KIND::DROPPED,
        pKirby->Get_KirbyAbility()->Get_AbilityType(),
        vSpawnPos, vDir, nullptr);
}

CKirby_AbilityDump* CKirby_AbilityDump::Create()
{
    CKirby_AbilityDump* pInstance = new CKirby_AbilityDump();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CKirby_AbilityDump");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_AbilityDump::Free()
{
    __super::Free();
}
