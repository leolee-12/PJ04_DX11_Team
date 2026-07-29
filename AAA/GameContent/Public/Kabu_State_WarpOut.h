#pragma once
#include "Monster_State.h"

NS_BEGIN(Client)
class CMonster;

class CKabu_State_WarpOut final : public CMonster_State
{
protected:
    CKabu_State_WarpOut() = default;
    virtual ~CKabu_State_WarpOut() = default;

protected:
    virtual HRESULT                 Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE      Get_StateType() override;
    virtual void                    Enter(MONSTER_STATE_TYPE ePrevState = MONSTER_STATE_TYPE::IDLE) override;
    virtual void                    Update(_float fTimeDelta) override;
    virtual void                    Exit(MONSTER_STATE_TYPE eNextState) override;

public:
    static CKabu_State_WarpOut*     Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

    static constexpr _float         s_fSpinDuration = 0.8f;
    static constexpr _float         s_fStartLead = 0.35f;
    static constexpr _float         s_fSpinMaxDegPerSec = 2880.f;
    static constexpr _float         s_fWarpInvisibleTime = 1.0f;

protected:
    _bool                           m_bStartFxPlayed = { false };
    _bool                           m_bEndFxPlayed = { false };
    _float4                         m_vSavedRight = {};
    _float4                         m_vSavedUp = {};
    _float4                         m_vSavedLook = {};

    virtual void                    Free() override;
};

NS_END

