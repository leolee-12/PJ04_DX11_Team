#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)
class CBoss_GorillaRush_Brain final : public CBoss_Brain
{
private:
    CBoss_GorillaRush_Brain() = default;
    virtual ~CBoss_GorillaRush_Brain() = default;
protected:
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;
public:
    static CBoss_GorillaRush_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};
NS_END