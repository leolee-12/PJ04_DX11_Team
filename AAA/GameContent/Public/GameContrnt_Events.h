#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

namespace Client
{
    typedef struct tagUIRButtonProbe {
        _float2 vNDC;
        _bool   bConsumed = false;
    }UI_RBTN_PROBE;

    typedef struct tagWorldRButtonDown {
        _float2 vNDC;
    }WORLD_RBTN_DOWN;

    enum class EClickHitType { NONE, FLOOR, ANIMAL, ITEMBOX };

    typedef struct tagWorldClickProbe {
        _float2        vNDC;
        _vector        vRayOrigin;
        _vector        vRayDir;

        EClickHitType  eHitType = EClickHitType::NONE;
        _float         fHitDist = FLT_MAX;
        CGameObject*   pHitObject = nullptr;
        _int           iHitSubIndex = -1;
        _float3        vHitPos = {};
    } WORLD_CLICK_PROBE;

    typedef struct tagWorldHoverProbe {
        _vector       vRayOrigin;
        _vector       vRayDir;

        _float        fHitDist = FLT_MAX;
        CGameObject*  pHitObject = nullptr;
        _int          iHitSubIndex = -1;
    } WORLD_HOVER_PROBE;

    typedef struct tagSkillInput {
        _vector vRayOrigin;
        _vector vRayDir;
        _bool   bHasRay = false;
    } SKILL_INPUT;

    typedef struct tagAttack {
        CGameObject* pVictim    = { nullptr };
        CGameObject* pAttacker  = { nullptr };
        _uint        iDmg       = { 0 };
    }ATTACK_DESC;

    typedef struct tagObjectDiedDesc {
        CGameObject* pObject = { nullptr };
    }COMBATANT_DIED_DESC;

    typedef struct tagObjectWillDestroy {
        CGameObject* pObject = { nullptr };
    }OBJECT_WILL_DESTROY_DESC;

    typedef struct tagShowItemBoxUI {
        _uint   iBoxIdx;
        _uint* pContents;    
    } SHOW_ITEMBOX_UI;

    typedef struct tagInventoryTryAdd {
        _uint   iItemID;
        _bool* pSuccess;
    } INVENTORY_TRY_ADD;

    typedef struct tagPlayerPosProbe {
        _float3* pOutPos = nullptr;
        _bool    bFilled = false;
    } PLAYER_POS_PROBE;

    typedef struct tagCraftStarted {
        _uint   iResultID;
        _float  fDuration;
    } CRAFT_STARTED_DESC;

    typedef struct tagCraftFinished {
        _uint   iResultID;
        _bool   bSuccess;    
    } CRAFT_FINISHED_DESC;

    typedef struct tagBossDefeated {
        CGameObject* pBoss = { nullptr };
	} BOSS_DEFEATED_DESC;
}