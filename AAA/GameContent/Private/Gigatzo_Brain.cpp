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

    CGigatzo* pGig = static_cast<CGigatzo*>(m_pOwner);

    // 접근 판정: Interact 반경 안 + 카메라 전방(전진코스터: 뒤쪽 억제)
    _bool bInRange = (nullptr != BlackBoard.pTarget) && pGig->Is_InCameraFront();

    if (!bInRange)
    {
        m_bArmed = false;          // 범위 밖 -> 재무장 대기
        return;
    }

    if (!m_bArmed)                 // 방금 진입 = 무장 (위상 = 접근 기준)
        {
            m_fFireTimer = pGig->Get_InitWaitDelay();
            m_bArmed = true;
      }

      m_fFireTimer -= fTimeDelta;
      if (m_fFireTimer > 0.f)
          return;
      m_fFireTimer += s_fFireInterval;

      if (!Can_Decide(BlackBoard))
          return;
      if (m_pOwner->Get_StateType() != MONSTER_STATE_TYPE::IDLE)
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
