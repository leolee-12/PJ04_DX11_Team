#pragma once
#include "Monster_Brain.h"

NS_BEGIN(Engine)
class CBehaviorTree;
class CBTNode;
NS_END

NS_BEGIN(Client)
class CBoss;

class CBoss_Brain abstract : public CMonsterBrain
{
protected:
    CBoss_Brain() = default;
    virtual ~CBoss_Brain() = default;

    HRESULT Initialize_Trees();

public:
    virtual void Decide(CMonster* pMonster, const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

protected:
    // 구체 보스 브레인이 페이즈별 트리를 만든다
    virtual _int     Get_PhaseCount() const = 0;
    virtual CBTNode* Build_PhaseTree(_int iPhase) = 0;

protected:
    vector<CBehaviorTree*> m_PhaseBTs;
    CBoss* m_pOwner = { nullptr };

    virtual void Free() override;
};
NS_END