#pragma once

#include "Base.h"
#include "Engine_Defines.h"

//-----------------------------------
//	충돌 처리 담당 매니저 클래스입니다.
//	콜라이더들을 해당 매니저에서 모아서 관리합니다.
//	충돌처리는 모두 여기서 합니다.
//-----------------------------------

// 콜백에서는 ImmediateUnregister말고 RequestUnregister 쓰기 즉시삭제는 위험함

NS_BEGIN(Engine)

class CGameObject;
class CCollider;

class ENGINE_DLL CCollision_Manager : public CBase
{
private:
	CCollision_Manager();
	virtual ~CCollision_Manager() = default;

public:
	//HRESULT Ready_CollisionMgr();													// 셋업함수
	void Add_CollisionPool(_uint SrcGroup, _uint DstGroup);
	void Clear_CollisionPool() { m_vecCollisionPool.clear(); }
	void Check_Collisions(const _float& fDeltaTime);								// 매프레임 충돌체크 함수
	void RequestUnregister(CCollider* pCollider, _uint Group);						// 삭제요청을 보내는 함수
	void RegisterCollider(CCollider* pCollider, _uint Group);						// 콜라이더 등록/갱신 함수
	void ImmediateUnregister(CCollider* pCollider, _uint Group);					// 콜라이더 정보 즉시 삭제 함수

	void Reset_For_SceneChange() { m_hmapCollisionGroup.clear(); }		// 씬전환용 등록된 충돌체 비우기 함수

	//vector<CGameObject*> Query_Collision(CCollider* pCollider, _uint Groupflag = UINT_MAX);	// 임시 충돌체크함수 (반환값 충돌한 모든 충돌체의 오너포인터 집단)

private:
	// 매니저 내부 데이터 관리 함수 모음 (해당 함수들은 외부에서 실행되면 안됨!!)
	void Execute_UnregisterRequests();											// 요청큐에 쌓은 삭제요청들을 한번에 정리하는 함수
	void UnregisterCollider(CCollider* pCollider, _uint Group);					// 콜라이더 해제/삭제 함수
	void Notify_Unregister(CCollider* pCollider);

private:
	typedef struct tagRequestInfo
	{
		CCollider* pCollider;
		_uint	 eGroup;
	}REQINFO;

	unordered_map<_uint, vector<CCollider*>> m_hmapCollisionGroup;		// 충돌체데이터 집합 (그룹별)
	vector<pair<_uint, _uint>> m_vecCollisionPool;						// 어떤그룹과 어떤그룹이 충돌하게할지 정하는 풀
	vector<REQINFO>			m_vecUnregisterRequestQueue;				// 삭제요청을 모아두는 컨테이너

public:
	static CCollision_Manager* Create();
	virtual void Free() override;
};

NS_END
