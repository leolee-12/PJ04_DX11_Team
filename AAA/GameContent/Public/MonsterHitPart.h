#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine) class CCollider; NS_END

NS_BEGIN(Client)

// MONSTER_HIT 히트박스를 가진 몬스터 파츠(칼/방패 등).
// Set_Drawn 으로 렌더+히트박스를 함께 토글, 닿으면 PLAYER_HURT에 데미지.
class CMonsterHitPart abstract : public CMonsterPart
{
    GENERATED_BODY_ABSTRACT(CMonsterHitPart)

protected:
    CMonsterHitPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMonsterHitPart(const CMonsterHitPart& Prototype);
    virtual ~CMonsterHitPart() = default;

public:
    virtual void Late_Update(_float fTimeDelta) override;
    void         Set_Drawn(_bool bDrawn);                   

protected:
    HRESULT Ready_HitBox(const CAPSULE_DESC& Desc, _float fDamage = 5.f, _float fKnockback = 8.f);

protected:
    CCollider* m_pHitBox = { nullptr };
    _float     m_fDamage = { 5.f };
    _float     m_fKnockback = { 8.f };

private:
    void SetUp_HitBox_Callback();

protected:
    virtual void Free() override;
};

NS_END