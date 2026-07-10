#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)
class IDeformable;

namespace EventTag
{
    inline constexpr const _tchar* Deform_Acquired = L"Deform.Acquired";
}

struct DEFORM_ACQUIRED_EVENT
{
    DEFORM_TYPE eType = { DEFORM_TYPE::NONE };
    IDeformable* pSource = { nullptr };
};

enum class DEFORM_OBJECT_KIND
{
    MOBILE,           // 오브젝트가 커비 쪽으로 이동·회전하며 흡수됨
    FIXED,            // 오브젝트는 고정, 커비가 오브젝트에 달라붙음
};

class IDeformable
{
protected:
    virtual ~IDeformable() = default;

public:
    virtual DEFORM_TYPE         Get_DeformType() const = 0;
    virtual DEFORM_OBJECT_KIND  Get_DeformKind() const = 0;

    virtual _bool   Request_Deform(const _float4x4* AnchorWorld) = 0;
    virtual void    End_Deform(const _float4x4* AnchorWorld) = 0;
};

NS_END