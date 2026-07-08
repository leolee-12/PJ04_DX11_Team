#pragma once
#include "GameContent_Defines.h"
#include "Base.h"

NS_BEGIN(Client)

class CLIENT_DLL CMissionManager final : public CBase
{
    DECLARE_SINGLETON(CMissionManager)

public:
    static constexpr _uint MAIN = 0;
    static constexpr _uint SUB_COUNT = 4;
    static constexpr _uint COUNT = 5;

private:
    CMissionManager() = default;
    ~CMissionManager() = default;

    struct MISSION_SLOT { _wstring strName; _bool bSucceeded = { false }; };
    MISSION_SLOT m_Slots[COUNT];

public:
    void Reset(const _wstring& strMainName = L"스테이지 클리어")
    {
        for (_uint i = 0; i < COUNT; ++i) { m_Slots[i].strName.clear(); m_Slots[i].bSucceeded = false; }
        m_Slots[MAIN].strName = strMainName;
    }
    void Bind_SubMission(_uint iSub, const _wstring& strName) { if (iSub < SUB_COUNT) m_Slots[1 + iSub].strName = strName; }
    void Set_Succeeded(_uint iSlot, _bool b) { if (iSlot < COUNT)    m_Slots[iSlot].bSucceeded = b; }
    void Report_SubSuccess(_uint iSub) { if (iSub < SUB_COUNT) m_Slots[1 + iSub].bSucceeded = true; }

    const _wstring& Get_Name(_uint i)   const { return m_Slots[i].strName; }
    _bool           Is_Succeeded(_uint i) const { return m_Slots[i].bSucceeded; }

public:
    void Free() {}
};
NS_END