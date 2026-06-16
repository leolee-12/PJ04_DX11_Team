#pragma once
#include "Base.h"
#include "BT_Defines.h"

NS_BEGIN(Engine)
class CBTNode;
class CBlackboard;

// Root + BlackBoard 보유, 매프레임 Tick
class ENGINE_DLL CBehaviorTree final : public CBase
{
private:
	CBehaviorTree() = default;
	virtual ~CBehaviorTree() = default;

public:
	BT_STATUS    Tick(_float fTimeDelta);
	CBlackboard* Get_Blackboard() const { return m_pBB; }


private:
	CBTNode*	 m_pRoot = { nullptr };
	CBlackboard* m_pBB   = { nullptr };

public:
	static CBehaviorTree* Create(CBTNode* pRoot);
	virtual void Free() override;
};

NS_END

