#include "Collision_Manager.h"
#include "GameObject.h"
#include "Collider.h"

CCollision_Manager::CCollision_Manager()
{
}

//HRESULT CCollision_Manager::Ready_CollisionMgr()
//{
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_MONSTER });		// 어떤 그룹끼리 충돌체크할 것인지 등록
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_ITEM });
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_GRASS });
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_WARP });
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_MBULLET });
//	m_vecCollisionPool.push_back({ CL_MONSTER, CL_GRASS });
//	m_vecCollisionPool.push_back({ CL_MONSTER, CL_PBULLET });
//	// 윤석현 추가
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_TRIGGER });
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_BORDER });
//	m_vecCollisionPool.push_back({ CL_MONSTER, CL_BORDER });
//	m_vecCollisionPool.push_back({ CL_PLAYER, CL_CHEST });
//	m_vecCollisionPool.push_back({ CL_BUILD, CL_BREAK });
//	m_vecCollisionPool.push_back({ CL_BUILD, CL_BUILD });
//	// WY
//	m_vecCollisionPool.push_back({ CL_NPC, CL_BORDER });
//
//	return S_OK;
//}

void CCollision_Manager::Add_CollisionPool(_uint SrcGroup, _uint DstGroup)
{
	m_vecCollisionPool.push_back({ SrcGroup, DstGroup });
}

void CCollision_Manager::Check_Collisions(const _float& fDeltaTime)
{
	for (auto& [grp, vec] : m_hmapCollisionGroup)
		for (auto* pC : vec) pC->Swap_ContactFrame();

	for (auto& pool : m_vecCollisionPool) {
		auto itA = m_hmapCollisionGroup.find(pool.first);
		auto itB = m_hmapCollisionGroup.find(pool.second);
		if (itA == m_hmapCollisionGroup.end() || itB == m_hmapCollisionGroup.end()) continue;

		_bool bSameGroup = (pool.first == pool.second);
		auto& vA = itA->second; auto& vB = itB->second;
		for (size_t i = 0; i < vA.size(); ++i) {
			if (!vA[i]->Is_Enabled()) continue;         
			size_t jStart = bSameGroup ? i + 1 : 0;
			for (size_t j = jStart; j < vB.size(); ++j) {
				if (vA[i] == vB[j]) continue;
				if (!vB[j]->Is_Enabled()) continue;       
				if (vA[i]->Intersect(vB[j])) {
					vA[i]->Add_Contact(vB[j]);
					vB[j]->Add_Contact(vA[i]);
				}
			}
		}
	}

	for (auto& [grp, vec] : m_hmapCollisionGroup)
		for (auto* pC : vec) pC->Dispatch_Callbacks();

	Execute_UnregisterRequests();
}

void CCollision_Manager::RegisterCollider(CCollider* pCollider, _uint Group)
{
	if (pCollider == nullptr) return;

	auto [iter, inserted] = m_hmapCollisionGroup.try_emplace(Group);

	if (!inserted) {
		for (auto* pExisting : iter->second)
			if (pCollider == pExisting) return;   // 이미 등록됨
	}

	iter->second.push_back(pCollider);
	pCollider->Mark_Registered(Group);
}

void CCollision_Manager::ImmediateUnregister(CCollider* pCollider, _uint Group)
{
	if (pCollider == nullptr) return;

	Notify_Unregister(pCollider);

	auto iter = m_hmapCollisionGroup.find(Group);
	if (iter == m_hmapCollisionGroup.end()) return;

	auto& vec = iter->second;
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if ((*it) == pCollider)
		{
			vec.erase(it);
			pCollider->Mark_Unregistered();
			return;
		}
	}
}

void CCollision_Manager::Query_Overlap(CCollider* pQuery, _uint Group, vector<CCollider*>* pOut)
{
	if (nullptr == pQuery || nullptr == pOut)
		return;

	auto iter = m_hmapCollisionGroup.find(Group);
	if (iter == m_hmapCollisionGroup.end())
		return;

	for (auto* pDst : iter->second)
	{
		if (nullptr == pDst || pDst == pQuery)
			continue;

		if (!pDst->Is_Enabled())
			continue;

		if (nullptr != pQuery->Get_Owner() && pQuery->Get_Owner() == pDst->Get_Owner())
			continue;

		if (pQuery->Intersect(pDst))
			pOut->push_back(pDst);
	}
}

void CCollision_Manager::Query_Overlap(CCollider* pQuery, const vector<_uint>& Groups, vector<CCollider*>* pOut)
{
	for (_uint Group : Groups)
		Query_Overlap(pQuery, Group, pOut);
}

void CCollision_Manager::Query_OverlapOwners(CCollider* pQuery, const vector<_uint>& Groups, vector<CGameObject*>* pOut)
{
	if (nullptr == pOut)
		return;

	vector<CCollider*> Hits;
	Query_Overlap(pQuery, Groups, &Hits);

	for (auto* pHit : Hits)
	{
		CGameObject* pOwner = pHit->Get_Owner();
		if (nullptr == pOwner)
			continue;

		if (find(pOut->begin(), pOut->end(), pOwner) != pOut->end())
			continue;

		pOut->push_back(pOwner);
	}
}

void CCollision_Manager::RequestUnregister(CCollider* pCollider, _uint Group)
{
	if (pCollider == nullptr) { return; }
	m_vecUnregisterRequestQueue.push_back({ pCollider, Group });
}

void CCollision_Manager::Execute_UnregisterRequests()
{
	if (m_vecUnregisterRequestQueue.empty()) { return; }
	for (auto& requestinfo : m_vecUnregisterRequestQueue)
	{
		UnregisterCollider(requestinfo.pCollider, requestinfo.eGroup);
	}
	m_vecUnregisterRequestQueue.clear();
}

void CCollision_Manager::UnregisterCollider(CCollider* pCollider, _uint Group)
{
	if (pCollider == nullptr) { return; }

	Notify_Unregister(pCollider);

	auto iter = m_hmapCollisionGroup.find(Group);
	if (iter == m_hmapCollisionGroup.end()) { return; }

	auto& vec = iter->second;
	// remove_if < 알고리즘 한번 찾아보세요.
	auto removeiter = remove_if(vec.begin(), vec.end(), [pCollider, this](const CCollider* pDstCollider) {
		if (pCollider == pDstCollider) {
			return true;											// 순회중에 삭제를 할 경우에 이미 순회/삭제 중인 컨테이너의 요소에 접근 할 수 있어서 반복자 무효화 발생할 수 있음
		}															// 해당하는 가능성을 제거하기위해 순회중에 삭제큐에 담아뒀다가 순회가 끝나면 한번에 삭제처리
		return false;
		});

	if (removeiter != vec.end())
	{
		vec.erase(removeiter, vec.end());
		pCollider->Mark_Unregistered();
	}
	// 삭제 큐를 여기서 비우지는 않는 이유:
	// Check_Collisions 에서 여기가 호출되면 컨테이너 순회 중 삭제가 발생하므로,
	// 안전하게 순회가 끝난(삭제 루프 밖)에서 ExecuteUnregister()로 처리한다.
}

void CCollision_Manager::Notify_Unregister(CCollider* pCollider)
{
	if (pCollider == nullptr) return;

	// 나랑 연관된(Prev, Curr) 모든 상대방에게 작별 인사를 합니다.
	unordered_set<CCollider*> AllContacts = pCollider->Get_PrevContacts();
	for (auto& pOther : pCollider->Get_CurrContacts())
		AllContacts.insert(pOther);

	for (auto& pOther : AllContacts)
	{
		if (pOther)
		{
			pOther->Notify_Exit(pCollider);       // 상대방에게 Exit 이벤트 발생
			pOther->Remove_FromContacts(pCollider); // 상대방의 리스트에서 나를 즉시 제거
		}
	}
}


//vector<CGameObject*> CCollision_Manager::Query_Collision(CCollider* pCollider, _uint Groupflag)
//{
//	vector<CGameObject*> vecTemp;			// 반환용 임시 벡터
//
//	for (auto& pair : m_hmapCollisionGroup)	// 충돌 그룹 전체순회
//	{
//		_uint group = pair.first;		// 충돌 그룹데이터 추출 (플래그와 비트연산을 위해)
//
//		// 요청한 레이어 그룹 해당 그룹이 포함되어 있지 않으면 건너뜀
//		if ((Groupflag & group) == 0)		// 비트 마스킹
//			continue;
//
//		for (const auto& pDstCollider : pair.second)		// 요청한 그룹 플래그에 해당 그룹이 포함되어있다면,
//		{													// 해당 그룹 전체 순회
//			if (pCollider->Intersect(pDstCollider))
//			{
//				pCollider->Notify_Enter(pDstCollider);
//				pDstCollider->Notify_Enter(pCollider);
//			}
//		}
//	}
//
//	return vecTemp;			// 충돌한 모든 충돌체의 게임포인터를 반환
//}
CCollision_Manager* CCollision_Manager::Create()
{
	return new CCollision_Manager();
}

void CCollision_Manager::Free()
{
	m_hmapCollisionGroup.clear();
}
