#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine) 
class CGameObject;
NS_END

NS_BEGIN(Client)

enum class HIT_TYPE : _uint
{
    NORMAL = 0,             
    BODY_CONTACT,
    SWORD_DEFAULT, SWORD_SPIN,
    CAR_BOOSTER_HIT,
};

struct ATTACK_INFO
{
    _float       fDamage = { 0.f };
    _float3      vAttackerPos = {};        
    _float       fKnockback = { 0.f };   // 넉백/발사 강도
    CGameObject* pAttacker = { nullptr };  // 팀/소유자 식별(선택)
    HIT_TYPE     eHitType = { HIT_TYPE::NORMAL };     // 일반/강공/투사체 (확장)
};

class IDamageable
{
public:
    virtual ~IDamageable() = default;
    virtual void Damaged(const ATTACK_INFO& tInfo) = 0;
};

NS_END