#include "BTComposite.h"

CBTComposite::CBTComposite(initializer_list<CBTNode*> Children)
{
	m_Children.reserve(Children.size());
	for (CBTNode* pChild : Children)
		m_Children.push_back(pChild);
}

void CBTComposite::Reset()
{
	m_iRunning = 0;
	for (CBTNode* pChild : m_Children)
		pChild->Reset();
}

void CBTComposite::Free()
{
	for (CBTNode* pChild : m_Children)
		Safe_Release(pChild);
	m_Children.clear();

	__super::Free();
}

BT_STATUS CBTSequence::Tick(CBlackboard* pBB, _float fDt)
{
	for (; m_iRunning < m_Children.size(); ++m_iRunning)
	{
		BT_STATUS eStatus = m_Children[m_iRunning]->Tick(pBB, fDt);

		if (eStatus == BT_STATUS::RUNNING)
			return BT_STATUS::RUNNING;                       // 이 자식에서 대기

		if (eStatus == BT_STATUS::FAILURE)
		{
			Reset();
			return BT_STATUS::FAILURE;                        // 하나라도 실패 → 실패
		}
		// SUCCESS → 다음 자식
	}

	Reset();
	return BT_STATUS::SUCCESS;				// 전부 성공
}

CBTSequence* CBTSequence::Create(initializer_list<CBTNode*> Children)
{
	return new CBTSequence(Children);
}

void CBTSequence::Free()
{
	__super::Free();
}

BT_STATUS CBTSelector::Tick(CBlackboard* pBB, _float fDt)
{
	for (; m_iRunning < m_Children.size(); ++m_iRunning)
	{
		BT_STATUS eStatus = m_Children[m_iRunning]->Tick(pBB, fDt);

		if (eStatus == BT_STATUS::RUNNING)
			return BT_STATUS::RUNNING;

		if (eStatus == BT_STATUS::SUCCESS)
		{
			Reset();
			return BT_STATUS::SUCCESS;                        // 하나라도 성공 → 성공
		}
		// FAILURE → 다음 후보
	}

	Reset();
	return BT_STATUS::FAILURE;
}

CBTSelector* CBTSelector::Create(initializer_list<CBTNode*> Children)
{
	return new CBTSelector(Children);
}

void CBTSelector::Free()
{
	__super::Free();
}


