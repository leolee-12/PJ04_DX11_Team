#include "Kirby_AttackMode.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

#include "Movement_Child.h"

CKirby_AttackMode::CKirby_AttackMode()
{
}

HRESULT CKirby_AttackMode::Initialize()
{
    m_pGameInstance_Proxy = CGameInstance::GetProxy();
    if (m_pGameInstance_Proxy == nullptr)
        return E_FAIL;

    return S_OK;
}

_bool CKirby_AttackMode::Can_Attack(KIRBY_ATTACK_LOCATION eAttackLocation)
{
    switch (eAttackLocation)
    {
    case KIRBY_ATTACK_LOCATION::GROUND:
    case KIRBY_ATTACK_LOCATION::AIR:
        return true;
    }

    return false;
}

void CKirby_AttackMode::On_Damaged_KirbyState(CKirby* pKirby, const ATTACK_INFO& tInfo)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Apply_Knockback(tInfo.vAttackerPos, 180.f, 9.f);  // Test

    pKirby->Add_HP(-tInfo.fDamage);
    pKirby->Start_DamageInvincibility();

    pKirby->Change_State(KIRBY_STATE_TYPE::DAMAGED);
}

void CKirby_AttackMode::Clear_Overlay(CKirby* pKirby, _uint iSlot, _float fOverlayBlendTime)
{
    pKirby->Get_Body()->Get_Animator()->Clear_Overlay(iSlot, fOverlayBlendTime);
}

void CKirby_AttackMode::Effect_Stop(CEffect_Container*& pContainer)
{
    if (pContainer != nullptr)
    {
        pContainer->EffectContainer_StopAfterEmission();
        pContainer = nullptr;
    }
}

void CKirby_AttackMode::Effect_StopImmediately(CEffect_Container*& pContainer)
{
    if (pContainer != nullptr)
    {
        pContainer->EffectContainer_Stop();
        pContainer = nullptr;
    }
}

void CKirby_AttackMode::Free()
{
    Safe_Release(m_pGameInstance_Proxy);

    __super::Free();
}
