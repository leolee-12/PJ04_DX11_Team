#pragma once
#include "Monster_State_Chase.h"

NS_BEGIN(Client)
class CCappy_State_Chase final : public CMonster_State_Chase
{
protected:
    CCappy_State_Chase() = default;
    virtual ~CCappy_State_Chase() = default;

protected:
    virtual HRESULT             Initialize(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f)  override;
    virtual void                Update(_float fTimeDelta) override;

public:
    static CCappy_State_Chase*  Create(const ANI_PLAY_INFO& tInfo = {}, _float fSpeed = 0.f);

protected:
    virtual void                Free() override;
};
NS_END