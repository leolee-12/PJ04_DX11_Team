#include "Monster_State_Spat.h"
#include "Monster.h"
#include "GameInstance.h"
#include "Animator.h"

HRESULT CMonster_State_Spat::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    return __super::Initialize(tInfo, fSpeed);
}

MONSTER_STATE_TYPE CMonster_State_Spat::Get_StateType() { return MONSTER_STATE_TYPE::SPAT; }

void CMonster_State_Spat::Enter(MONSTER_STATE_TYPE ePrevState)
{
    if (!m_pOwner || !m_pAnimator)
        return;

    // 비행/프로젝타일박스는 CSpit_Projectile 소유. 여기선 애님만 재생
    if (!m_PlayInfo.strAniName.empty())
        m_pAnimator->Play(&m_PlayInfo);
}

void CMonster_State_Spat::Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);
}

void CMonster_State_Spat::Exit(MONSTER_STATE_TYPE eNextState)
{
    UNREFERENCED_PARAMETER(eNextState);
}

CMonster_State_Spat* CMonster_State_Spat::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CMonster_State_Spat* pInstance = new CMonster_State_Spat();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CMonster_State_Spat"); Safe_Release(pInstance);
    }
    return pInstance;
}


void CMonster_State_Spat::Free() 
{
    __super::Free();
}