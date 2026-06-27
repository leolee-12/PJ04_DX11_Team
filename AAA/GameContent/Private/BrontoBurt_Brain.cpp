#include "BrontoBurt_Brain.h"
#include "BrontoBurt.h"
#include "Monster_RailMovement.h"

CBrontoBurt_Brain::CBrontoBurt_Brain()
{
}

HRESULT CBrontoBurt_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CBrontoBurt_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

    if (!Can_Decide(BlackBoard))
        return;

    UNREFERENCED_PARAMETER(fTimeDelta);

    if (m_pOwner->Get_StateType()
        != MONSTER_STATE_TYPE::IDLE)
        return;

    if (!m_pOwner->Has_State(MONSTER_STATE_TYPE::RETURN))
        return;

    auto pRail = static_cast<CMonster_RailMovement*>(m_pOwner->Get_Movement());
    if (nullptr == pRail || !pRail->Has_Rail())
        return;

    if (pRail->Is_OffPath())
        m_pOwner->Change_State(MONSTER_STATE_TYPE::RETURN);
}

CBrontoBurt_Brain* CBrontoBurt_Brain::Create(CMonster* pOwner)
{
    CBrontoBurt_Brain* pInstance = new CBrontoBurt_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CBrontoBurt_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBrontoBurt_Brain::Free()
{
    __super::Free();
}
