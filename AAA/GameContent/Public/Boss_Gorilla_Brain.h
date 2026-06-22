#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Gorilla_Brain final : public CBoss_Brain
{
private:
    CBoss_Gorilla_Brain() = default;
    virtual ~CBoss_Gorilla_Brain() = default;

protected:
    virtual _int     Get_PhaseCount() const override { return 3; }   // ¡Ú s_Thresholds.size()+1 °ú ÀÏÄ¡
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

public:
    static CBoss_Gorilla_Brain* Create();
};

NS_END