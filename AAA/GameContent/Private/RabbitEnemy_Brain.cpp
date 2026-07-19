#include "RabbitEnemy_Brain.h"
#include "Monster.h"

CRabbitEnemy_Brain::CRabbitEnemy_Brain()
{
}

HRESULT CRabbitEnemy_Brain::Initialize(CMonster* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CRabbitEnemy_Brain::Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (nullptr == m_pOwner)
        return;

    if (m_pOwner->Get_StateType() != MONSTER_STATE_TYPE::IDLE)
        return;

    if (nullptr == BlackBoard.pTarget)
        return;

    if (m_pOwner->Has_State(MONSTER_STATE_TYPE::JUMP))
        m_pOwner->Change_State(MONSTER_STATE_TYPE::JUMP);
}

CRabbitEnemy_Brain* CRabbitEnemy_Brain::Create(CMonster* pOwner)
{
    CRabbitEnemy_Brain* pInstance = new CRabbitEnemy_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CRabbitEnemy_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CRabbitEnemy_Brain::Free()
{
    __super::Free();
}
