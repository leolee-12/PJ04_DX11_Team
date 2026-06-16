#pragma once
#include "Monster_Brain.h"

NS_BEGIN(Engine)
class CBehaviorTree;
NS_END

NS_BEGIN(Client)
class CGigantEdge;

class CGigantEdge_Brain final : public CMonsterBrain
{
private:
    CGigantEdge_Brain() = default;
    virtual ~CGigantEdge_Brain() = default;
    HRESULT Initialize();

public:
    virtual void Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static CGigantEdge_Brain* Create();
    virtual void Free() override;

private:
    CBehaviorTree* m_pBT = { nullptr };
    CGigantEdge* m_pOwner = { nullptr };
};
NS_END