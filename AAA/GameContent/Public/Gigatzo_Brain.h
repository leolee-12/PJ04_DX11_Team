#pragma once
#include "Monster_Brain_FSM.h"

NS_BEGIN(Client)

class CGigatzo_Brain final : public CMonster_Brain_FSM
{
protected:
	CGigatzo_Brain();
	virtual ~CGigatzo_Brain() = default;

protected:
    virtual HRESULT                 Initialize(CMonster* pOwner) override;
    virtual void                    Decide(const MONSTER_BLACKBOARD& BlackBoard, _float fTimeDelta) override;

public:
    static  CGigatzo_Brain*         Create(CMonster* pOwner);

private:
    _bool                           m_bArmed = { false };
    _bool                           m_bPassLatched = { false };
    _float                          m_fFireTimer = { 0.f };
    _float3                         m_vPrevTargetPos = {};
    _bool                           m_bHasPrevTarget = { false };

    _float                          m_fPrevTargetSpeed = { 0.f };
    _float                          m_fTargetAccel = { 0.f };
    _float                          m_fObservedMaxSpeed = { 0.f };
    _bool                           m_bHasPrevSpeed = { false };

    static constexpr const _float   s_fFireInterval = { 1.93f };

    static constexpr const _float   s_fAnimDelay = { 0.30f };
    static constexpr const _float   s_fLeadBias = { 2.0f };
    static constexpr const _float   s_fRearmDistance = { 5.f };

    static constexpr const _float   s_fTeleportStep = { 20.f };

    static constexpr const _float   s_fMinLeadSpeed = { 12.f };

    static constexpr const _float   s_fMaxTargetAccel = { 150.f };
    static constexpr const _float   s_fAccelSmoothRate = { 10.f };

protected:
    virtual void                    Free() override;

};

NS_END