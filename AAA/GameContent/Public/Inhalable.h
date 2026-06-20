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
};

NS_END