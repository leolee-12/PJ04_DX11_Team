#include "Noddy_Brain.h"
#include "Monster.h"

CNoddy_Brain::CNoddy_Brain()
{
}

HRESULT CNoddy_Brain::Initialize(CMonster* pOwner)
{
    if (FAILED(__super::Initialize(pOwner)))
        return E_FAIL;

    return S_OK;
}

void CNoddy_Brain::Decide_Internal(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (nullptr == m_pOwner)
        return;

    if (m_pOwner->Get_StateType() == MONSTER_STATE_TYPE::IDLE
        && nullptr != BlackBoard.pTarget
        && m_pOwner->Has_State(MONSTER_STATE_TYPE::DETECT))
        m_pOwner->Change_State(MONSTER_STATE_TYPE::DETECT);
}

CNoddy_Brain* CNoddy_Brain::Create(CMonster* pOwner)
{
    CNoddy_Brain* pInstance = new CNoddy_Brain();
    if (FAILED(pInstance->Initialize(pOwner)))
    {
        MSG_BOX("Failed to Created : CNoddy_Brain");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNoddy_Brain::Free()
{
    __super::Free();
}
