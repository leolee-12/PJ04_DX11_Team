#pragma once
#include "Base.h"
#include "BT_Defines.h"
#include <functional>

NS_BEGIN(Engine)
class CBlackboard;

class ENGINE_DLL CBTNode abstract : public CBase
{
protected:
	CBTNode() = default;
	virtual ~CBTNode() = default;

public:
	virtual BT_STATUS Tick(CBlackboard* pBB, _float fTimeDelta) = 0;
	virtual void	  Reset() {}
};

// 리프 노드: 람다 액션
class ENGINE_DLL CBTAction final : public CBTNode
{
public:
	using TickFn = function<BT_STATUS(CBlackboard*, _float)>;
	using ResetFn = function<void()>;

	virtual BT_STATUS Tick(CBlackboard* pBB, _float fDt) override
	{
		return m_Fn(pBB, fDt);
	}

	virtual void Reset() override
	{
		if (m_ResetFn) m_ResetFn();
	}

private:
	TickFn  m_Fn;
	ResetFn m_ResetFn;

public:
	static CBTAction* Create(TickFn fn, ResetFn rfn = nullptr);
	virtual void Free() override;
};

// 리프 노드: 람다 조건 (true = 성공 / flase = 실패)
class ENGINE_DLL CBTCondition final : public CBTNode
{
public:
	using CondFn = function<bool(CBlackboard*)>;
	virtual BT_STATUS Tick(CBlackboard* pBB, _float fDt) override
	{
		return m_Fn(pBB) ? BT_STATUS::SUCCESS : BT_STATUS::FAILURE;
	}

private:
	CondFn m_Fn;

public:
	static CBTCondition* Create(CondFn fn);
	virtual void Free() override;
};

NS_END

