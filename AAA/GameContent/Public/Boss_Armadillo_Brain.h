#pragma once
#include "Boss_Brain.h"

NS_BEGIN(Client)

class CBoss_Armadillo_Brain final : public CBoss_Brain
{
private:
    CBoss_Armadillo_Brain() = default;
    virtual ~CBoss_Armadillo_Brain() = default;

protected:
    // 보스러쉬용: 페이즈 구분 없음
    virtual _int     Get_PhaseCount() const override { return 1; }
    virtual CBTNode* Build_PhaseTree(_int iPhase) override;

private:
    _bool m_bOpeningDone = { false };   // 조우 직후 오프닝 시퀀스 1회 소진 여부

public:
    static CBoss_Armadillo_Brain* Create(CMonster* pOwner);
    virtual void Free() override;
};

NS_END