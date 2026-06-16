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

BT_STATUS CBTReactiveSelector::Tick(CBlackboard* pBB, _float fDt)
{
	for (_uint i = 0; i < m_Children.size(); ++i)
	{
		BT_STATUS eStatus = m_Children[i]->Tick(pBB, fDt);

		if (eStatus == BT_STATUS::FAILURE)
			continue;                       // 다음(더 낮은 우선순위) 후보로

		// i가 이번 틱 승자. 직전에 다른 자식이 돌고 있었으면 그 브랜치 중단
		if (m_iRunning != INVALID_IDX && m_iRunning != i)
			m_Children[m_iRunning]->Reset();

		m_iRunning = (eStatus == BT_STATUS::RUNNING) ? i : INVALID_IDX;
		return eStatus;                     // SUCCESS여도 다음 틱 0부터 재평가됨
	}

	// 전부 FAILURE
	if (m_iRunning != INVALID_IDX)
	{
		m_Children[m_iRunning]->Reset();
		m_iRunning = INVALID_IDX;
	}
	return BT_STATUS::FAILURE;
}

void CBTReactiveSelector::Reset()
{
	__super::Reset();           // 자식 전부 Reset (base가 m_iRunning=0으로 둠)
	m_iRunning = INVALID_IDX;
}

CBTReactiveSelector* CBTReactiveSelector::Create(initializer_list<CBTNode*> C)
{
	return new CBTReactiveSelector(C);
}

void CBTReactiveSelector::Free()
{
	__super::Free();
}


