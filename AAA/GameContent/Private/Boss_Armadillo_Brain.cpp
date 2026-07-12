#include "Boss_Armadillo_Brain.h"
#include "Boss.h"
#include "BT.h"
#include "Animator.h"
#include "Boss_Armadillo.h"
#include "Boss_Armadillo_Body.h"

CBTNode* CBoss_Armadillo_Brain::Build_PhaseTree(_int iPhase)
{
    auto Anim = [this]() -> CAnimator* { return m_pOwner->Get_BodyAnimator(); };

    // TODO: 공격 패턴(구르기, 팔 휘두르기 등) 채우기. 지금은 대기 루프만.
    return CBTPlayClip::Create(Anim, { "Wait", true, 1.f, 1.f });
}

CBoss_Armadillo_Brain* CBoss_Armadillo_Brain::Create(CMonster* pOwner)
{
    CBoss_Armadillo_Brain* pInstance = new CBoss_Armadillo_Brain();
    if (FAILED(pInstance->Initialize_Trees(pOwner)))
    {
        MSG_BOX("Failed to Created : CBoss_Armadillo_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Armadillo_Brain::Free()
{
    __super::Free();
}