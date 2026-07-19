#include "Gigatzo_Brain.h"
#include "Gigatzo.h"

CGigatzo_Brain::CGigatzo_Brain()
{
}

HRESULT CGigatzo_Brain::Initialize(CMonster* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CGigatzo_Brain::Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta)
{
    if (nullptr == m_pOwner)
        return;

    if (m_bInit)
    {
        m_fFireTimer = static_cast<CGigatzo*>(m_pOwner)->Get_InitWaitDelay();
        m_bInit = false;
    }

    m_fFireTimer -= fTimeDelta;             // 감지·상태 무관 자유진행(위상 보존)
    if (m_fFireTimer > 0.f)
        return;
    m_fFireTimer += s_fFireInterval;        // '=' 아니라 '+=' (위상 유지)

    if (!Can_Decide(BlackBoard))
        return;
    if (m_pOwner->Get_StateType() != MONSTER_STATE_TYPE::IDLE)
        return;
    if (nullptr == BlackBoard.pTarget)
        return;

    m_pOwner->Change_State(MONSTER_STATE_TYPE::ATTACK);
}

CGigatzo_Brain* CGigatzo_Brain::Create(CMonster* pOwner)
{
	CGigatzo_Brain* pInstance = new CGigatzo_Brain();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CGigatzo_Brain");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGigatzo_Brain::Free()
{
	__super::Free();
}
