#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

class CLIENT_DLL ICurtainPart
{
public:
    virtual ~ICurtainPart() = default;
    virtual void    Begin_Delayed() = 0;    
    virtual void    Anim_Stop() = 0;
    virtual _bool   Is_Finished() const = 0;
    virtual _bool   Is_Loop()     const = 0;
    virtual void    Reset() = 0;
};

NS_END