#include "BTDecorator.h"
#include "BehaviorTree.h"

CBTDecorator::CBTDecorator(CBTNode* pChild)
	: m_pChild(pChild)
{
}

void CBTDecorator::Reset()
{
	if (nullptr != m_pChild)
		m_pChild->Reset();
}

void CBTDecorator::Free()
{
	Safe_Release(m_pChild);
}



BT_STATUS CBTInverter::Tick(CBlackboard* pBB, _float fDt)
{
	BT_STATUS eStatus = m_pChild->Tick(pBB, fDt);

	if (eStatus == BT_STATUS::SUCCESS) return BT_STATUS::FAILURE;
	if (eStatus == BT_STATUS::FAILURE) return BT_STATUS::SUCCESS;
	return BT_STATUS::RUNNING;            // RUNNING은 그대로
}

CBTInverter* CBTInverter::Create(CBTNode* pChild)
{
	return new CBTInverter(pChild);
}

void CBTInverter::Free()
{
	__super::Free();
}



BT_STATUS CBTCooldown::Tick(CBlackboard* pBB, _float fDt)
{
	if (m_fElapsed < m_fCooldown)         // 쿨다운 중
	{
		m_fElapsed += fDt;
		return BT_STATUS::FAILURE;
	}

	BT_STATUS eStatus = m_pChild->Tick(pBB, fDt);

	if (eStatus == BT_STATUS::SUCCESS)
		m_fElapsed = 0.f;                 // 성공 시 쿨다운 시작

	return eStatus;
}

void CBTCooldown::Reset()
{
	__super::Reset();                     
	m_fElapsed = m_fCooldown;
}

CBTCooldown* CBTCooldown::Create(CBTNode* pChild, _float fCooldown)
{
	CBTCooldown* pInstance = new CBTCooldown(pChild, fCooldown);
	pInstance->m_fElapsed = fCooldown;    // 처음엔 바로 사용 가능
	return pInstance;
}

void CBTCooldown::Free()
{
	__super::Free();
}
