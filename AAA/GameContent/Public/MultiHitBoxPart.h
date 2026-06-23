#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine) 
class CCollider; 
NS_END

NS_BEGIN(Client)

class CMultiHitBoxPart abstract : public CMonsterPart
{
    GENERATED_BODY_ABSTRACT(CMultiHitBoxPart)

protected:
    CMultiHitBoxPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CMultiHitBoxPart(const CMultiHitBoxPart& Prototype);
    virtual ~CMultiHitBoxPart() = default;

public:
    virtual void Late_Update(_float fTimeDelta) override;

    void Enable_HitBox(_int iIndex, _bool b);
    void Enable_AllHitBoxes(_bool b);
    _int Get_HitBoxCount() const { return (_int)m_HitBoxes.size(); }

protected:
    HRESULT Add_HitBox(const _char* szBone, COLLIDER eShape,
        _float fRadius, _float fHeight,
        _float fDamage, _float fKnockback,
        const _float3& vCenter = { 0.f, 0.f, 0.f },
        const _float3& vRadians = { 0.f, 0.f, 0.f });

private:
    struct HITBOX
    {
        CCollider* pCollider = { nullptr };
        const _float4x4* pBoneMatrix = { nullptr };
        _float           fDamage = { 0.f };
        _float           fKnockback = { 0.f };
        unordered_set<class CGameObject*> setHit;
    };

    void SetUp_HitBox_Callback(_int iIndex);

    vector<HITBOX> m_HitBoxes;

protected:
    virtual void Free() override;
};

NS_END