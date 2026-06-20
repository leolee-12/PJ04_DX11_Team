#pragma once
#include "Monster_StateT.h"

NS_BEGIN(Client)
class CMonster;

class CMonster_State_Spat final : public CMonster_StateT<CMonster>
{
private:
    CMonster_State_Spat() = default;
    virtual ~CMonster_State_Spat() = default;

protected:
    virtual HRESULT             Initialize() override;
    virtual HRESULT             Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed = 0.f) override;

public:
    virtual MONSTER_STATE_TYPE  Get_StateType() override;
    virtual void               On_Enter(CMonster* pMonster) override;
    virtual void               On_Update(CMonster* pMonster, _float fTimeDelta) override;
    virtual void               On_Exit(CMonster* pMonster) override;

private:
    _float                     m_fLifeTime = { 0.f };
    static constexpr _float     s_fMaxLifeTime = 2.5f;
    static constexpr _float     s_fSpinSpeedDeg = 360.f;

public:
    static CMonster_State_Spat* Create(const ANI_PLAY_INFO& tInfo, _float fSpeed = 0.f);
    static CMonster_State_Spat* Create();

protected:
    virtual void               Free() override;
};
NS_END