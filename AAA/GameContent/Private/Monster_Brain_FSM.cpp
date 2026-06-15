#include "Monster_Brain_FSM.h"

CMonster_Brain_FSM::CMonster_Brain_FSM()
{
}

HRESULT CMonster_Brain_FSM::Initialize()
{
	return S_OK;
}

void CMonster_Brain_FSM::Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
}

CMonster_Brain_FSM*  CMonster_Brain_FSM::Create()
{
    CMonster_Brain_FSM* pInstance = new CMonster_Brain_FSM();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created: CMonster_Brain_FSM");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMonster_Brain_FSM::Free()
{
    __super::Free();
}
