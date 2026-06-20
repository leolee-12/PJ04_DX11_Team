#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)

class CMonster;

// 공용 StateMachine은 CMonster*를 넘긴다.
// CMonster_StateT 한겹이 캐스팅을 딱 1회 처리해서 
// 파생 상태는 타입(T*)이 박힌 On_* 훅만 구현하면 된다.

template<class T>
class CMonster_StateT : public CMonster_State
{
protected:
	CMonster_StateT() = default;
	virtual ~CMonster_StateT() = default;

protected:
	// 파생 상태가 구현하는 훅들 (캐스팅 불필요, 컴파일러가 타입 검증)
	virtual void			On_Enter(T*) {}
	virtual void			On_Update(T*, _float) {}
	virtual void			On_Exit(T*) {}

private:
	virtual void			Enter(CMonster* pMonster) final
	{
		On_Enter(static_cast<T*>(pMonster));
	}
	virtual void			Update(CMonster* pMonster, _float fTimeDelta) final
	{
		On_Update(static_cast<T*>(pMonster), fTimeDelta);
	}
	virtual void			Exit(CMonster* pMonster) final
	{
		On_Exit(static_cast<T*>(pMonster));
	}

};

NS_END