#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine) 
class CGameObject; 
NS_END

NS_BEGIN(Client)

struct INHALE_QUERY
{
    _bool                bSuperInhale = { false };   // 강한 흡입인가
    CGameObject*         pInhaler = { nullptr };
};

class IInhalable
{
public:
    virtual ~IInhalable() = default;
    virtual _bool Can_BeInhaled(const INHALE_QUERY& q) const = 0;
    virtual void  Be_Captured(CGameObject* pInhaler) = 0;
    virtual COPY_ABILITY_TYPE Get_CopyAbility() const = 0;
    virtual CGameObject* Get_GameObject() = 0;
    virtual _float3 Get_SpatPivotOffset() const { return _float3(0.f, 0.f, 0.f); }

    virtual void  On_SpatBegin() = 0; // AI/컨트롤러/자체콜라이더 끄고 애님 전환
    virtual void  On_SpatEnd() = 0; // 풀 반환(Set_Active false) 또는 사망FX
};

NS_END