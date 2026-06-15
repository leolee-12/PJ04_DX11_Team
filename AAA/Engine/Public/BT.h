#pragma once
#include "BT_Defines.h"
#include "BlackBoard.h"
#include "BTNode.h"
#include "BTComposite.h"
#include "BTDecorator.h"
#include "BehaviorTree.h"


// 게임 컨텐츠에서는 해당 헤더 포함하기
// 
//- adopt 모델 : Create에 넘긴 노드는 그 부모(컴포지트 / 데코 / 트리)가 소유.호출측은 따로 Release 안 함.
//- 트리 통째 파괴 : Safe_Release(m_pBT) 한 번 → 루트→자식→손자 재귀적으로 Free 전파.
//- 한 노드를 두 부모에 넣지 말 것(이중 Release).공유가 필요하면 그 자리에서 Create로 새로 만들거나, 넣을 때 Safe_AddRef 후 각자
//  Release하는 변형이 필요(기본 골격은 단일 소유 가정).

// 기본 set / get
// 
// CBlackboard * bb = m_pBT->Get_Blackboard();
// 
// 저장
// bb->Set<_float>("hpRatio", 0.8f);
// bb->Set<_int>("phase", 2);
// bb->Set<_bool>("isEnraged", false);
// 
// 읽기 (없으면 두 번째 인자=기본값)
// _float hp = bb->Get<_float>("hpRatio", 1.f);
// _int   phase = bb->Get<_int>("phase", 0);
// _bool  rage = bb->Get<_bool>("isEnraged", false);
// 
// 객체 포인터 - void* 로 넣고 캐스팅
// 
// bb->Set<void*>("target", pPlayer);                 넣을 때 void*
// // ...
// auto* pTarget = static_cast<CGameObject*>(
//     bb->Get<void*>("target", nullptr));            꺼내서 캐스팅
// if (pTarget) { /* ... */ }
// 
// 벡터 / 위치
// 
// bb->Set<_float3>("lastPlayerPos", vPlayerPos);
// _float3 p = bb->Get<_float3>("lastPlayerPos", _float3(0, 0, 0));
// 
// 있는지 확인 / 지우기
// 
// if (bb->Has("target")) { ... }
// bb->Erase("target");