#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Gorilla_Brain final : public CBoss_Brain
{
private:
    CBoss_Gorilla_Brain() = default;
    virtual ~CBoss_Gorilla_Brain() = default;

protected:
    virtual _int     Get_PhaseCount() const override { return 2; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    _int   m_iMeleeSinceThrow = { 0 };
    _float m_fAtkTimer = { 0.f };

public:
    static CBoss_Gorilla_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END