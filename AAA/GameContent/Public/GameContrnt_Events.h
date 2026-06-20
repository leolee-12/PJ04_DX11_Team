#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

namespace Client
{
    class CMonster;

    // 호준 테스트용
    namespace EventTag
    {
        inline constexpr const _tchar* Kirby_PointStarGained = L"Kirby.PointStarGained";
        inline constexpr const _tchar* Kirby_HP_Updated = L"Kirby.HPUpdated";
        inline constexpr const _tchar* Kirby_Ability_Changed = L"Kirby.AbilityChanged";
    }

    inline constexpr const _tchar* EVT_SWALLOWED = L"OnSwallowed";
    inline constexpr const _tchar* EVT_QUERY_PLAYER = L"Query_Player";

    struct PLAYER_QUERY 
    { 
        CGameObject* pPlayer = { nullptr }; 
    };

    struct SWALLOW_EVENT 
    { 
        CMonster* pMonster; 
    };

    typedef struct tagKirbyPointStarGained
    {
        _uint iAmount = 1;
    }KIRBY_POINTSTAR_GAINED_DESC;

    typedef struct tagKirbyHPUpdated
    {
        _float fMaxHP = { 100.f };
        _float fCurrHp = { 100.f };
    }KIRBY_HP_UPDATED;

}