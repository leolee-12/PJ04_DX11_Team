#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Armadillo_Brain final : public CBoss_Brain
{
private:
    CBoss_Armadillo_Brain() = default;
    virtual ~CBoss_Armadillo_Brain() = default;

protected:
    // s_Thresholds.size() + 1 과 일치해야 함 (현재 1페이즈)
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

public:
    static CBoss_Armadillo_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END