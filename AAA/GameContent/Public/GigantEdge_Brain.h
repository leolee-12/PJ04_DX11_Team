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
    HRESULT Initialize(CMonster* pOwner);

public:
    virtual void Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static CGigantEdge_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
    
    CGigantEdge* Owner() const;

private:
    CBehaviorTree* m_pBT = { nullptr };
};
NS_END