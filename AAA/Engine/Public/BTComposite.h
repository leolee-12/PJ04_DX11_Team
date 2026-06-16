#pragma once
#include "BTNode.h"

NS_BEGIN(Engine)
class CBlackboard;


class ENGINE_DLL CBTComposite abstract : public CBTNode
{
protected:
	CBTComposite(initializer_list<CBTNode*> Children);
	virtual ~CBTComposite() = default;

public:
	virtual void Reset() override;

protected:
	vector<CBTNode*> m_Children;
	_uint			 m_iRunning = { 0 };

public:
	virtual void Free() override;
};

// 전부 성공이면 성공, 전부 실패면 실패
class ENGINE_DLL CBTSequence final : public CBTComposite
{
private:
	CBTSequence(initializer_list<CBTNode*> Children) : CBTComposite(Children) {}
	virtual ~CBTSequence() = default;

public:
	BT_STATUS Tick(CBlackboard* pBB, _float fDt) override;

public:
	static CBTSequence* Create(initializer_list<CBTNode*> Children);
	virtual void Free() override;
};

class ENGINE_DLL CBTSelector final : public CBTComposite
{
private:
	CBTSelector(initializer_list<CBTNode*> Children) : CBTComposite(Children) {}
	virtual ~CBTSelector() = default;

public:
	BT_STATUS Tick(CBlackboard* pBB, _float fDt) override;

public:
	static CBTSelector* Create(initializer_list<CBTNode*> Children);
	virtual void Free() override;
};


// 반응형(우선순위) 셀렉터: 매 틱 0번부터 재평가
// 더 높은 우선순위 자식이 이기면, 직전에 러닝이던 잣식을 Reset()으로 중단
class ENGINE_DLL CBTReactiveSelector final : public CBTComposite
{
private:
	CBTReactiveSelector(initializer_list<CBTNode*> Children) : CBTComposite(Children) { m_iRunning = INVALID_IDX; }
	virtual ~CBTReactiveSelector() = default;

	static constexpr _uint INVALID_IDX = 0xffffffff;

public:
	virtual BT_STATUS Tick(CBlackboard* pBB, _float fDt) override;
	virtual void      Reset() override;

public:
	static CBTReactiveSelector* Create(initializer_list<CBTNode*> Children);
	virtual void Free() override;
};

NS_END

