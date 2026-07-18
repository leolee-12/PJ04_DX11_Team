#pragma once
#include "GameContent_Defines.h"
#include "Damageable.h"

NS_BEGIN(Client)

enum class LD_CATEGORY : _uint
{
	PORTAL,
	DOOR,
	RAIL,
	VOLUME,
	GUIDE_AREA,
	AUDIO_AREA,
	ITEM,
	FOOD,
	BREAKABLE,
	FOLIAGE,
	ENEMY,
	GIMMICK,
	META,
	UNSUPPORTED,
	END
};

inline constexpr HIT_TYPE g_LDCutHitTypes[] =
{
      HIT_TYPE::SWORD_DEFAULT,
      HIT_TYPE::SWORD_SPIN,
      HIT_TYPE::UPWARD_SLASH
};

inline constexpr _bool Is_CutHit(HIT_TYPE eHitType)
{
    for (HIT_TYPE eCutHitType : g_LDCutHitTypes)
    {
        if (eCutHitType == eHitType)
            return true;
    }

    return false;
}

struct LD_CONTACT_HIT_RULE
{
    COLLISION_LAYER eLayer;
    HIT_TYPE eHitType;
    _bool bRequirePlayerOwner;
};

inline constexpr LD_CONTACT_HIT_RULE g_LDContactHitRules[] =
{
      { COLLISION_LAYER::PLAYER_BREAKERABLE, HIT_TYPE::BREAKERABLE_HIT, false },
      { COLLISION_LAYER::PLAYER_BOMB, HIT_TYPE::BOMB, false },
      { COLLISION_LAYER::PLAYER_HIT, HIT_TYPE::SLIDE, true }
};

inline _bool Try_ResolveContactHit(_uint iGroup, CGameObject* pAttacker, CGameObject* pPlayer, ATTACK_INFO* pOutAttackInfo)
{
    if (nullptr == pOutAttackInfo)
        return false;

    for (const LD_CONTACT_HIT_RULE& Rule : g_LDContactHitRules)
    {
        if (ETOUI(Rule.eLayer) != iGroup)
            continue;

        if (Rule.bRequirePlayerOwner && (nullptr == pPlayer || pAttacker != pPlayer))
            return false;

        *pOutAttackInfo = {};
        pOutAttackInfo->pAttacker = pAttacker;
        pOutAttackInfo->eHitType = Rule.eHitType;
        return true;
    }

    return false;
}

NS_END