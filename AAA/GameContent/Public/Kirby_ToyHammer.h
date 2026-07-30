#pragma once

#include "Kirby_OnOffPart.h"

#include "Damageable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

enum class TOY_HAMMER_HITBOX_TYPE
{
    HAMMER_ATTACK, HAMMER_ATTACK_FINAL,
    CHARGE_ATTACK_1, CHARGE_ATTACK_2, CHARGE_ATTACK_3, CHARGE_ATTACK_4,
    WHEELHAMMER, WHEELHAMMER_FALL
};

class CKirby_ToyHammer final : public CKirby_OnOffPart
{
    GENERATED_BODY(CKirby_ToyHammer)

private:
    enum TOY_HAMMER_MESH { HANDLE, HEAD, TOP, MESH_END };

public:
    struct KIRBY_TOYHAMMER_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_ToyHammer";
    static constexpr const wchar_t* Kirby_PartTag = L"ToyHammer";

private:
    CKirby_ToyHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CKirby_ToyHammer(const CKirby_ToyHammer& Prototype);
    virtual ~CKirby_ToyHammer() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual void Set_PartMode(CKirby* pKirby, KIRBY_PART_MODE ePartMode) override;

public:
    void Begin_Hit(const ATTACK_INFO& tInfo, _bool bResetHitList = true);
    void End_Hit(_bool bResetHitList = true);

    void Reset_DamagedList() { m_DamagedTargets.clear(); }

    void Set_HitBox(_bool bOn);

public:
    void BurnHammer(_bool bBurn) { m_bBurn = bBurn; }
    void Change_HitBox(TOY_HAMMER_HITBOX_TYPE eHitBoxType);
    _bool Is_HammerHeadCollision(_float fNormalY = 0.f, _float3* pOutPos = nullptr, _float3* pOutNormal = nullptr);

    const _float4x4* Get_HammerHeadWorldMatrixPtr() { return &m_HammerHeadWorldMatrix; }

private:
    HRESULT Ready_Components();
    HRESULT Ready_HitBox();
    void	SetUp_HitBox_Callback();
    virtual _bool Should_RenderShadowMesh(_uint iMeshIndex) override;

    void Cal_HammerHeadEffectSocket();

private:
    CCollider* m_pHitBox{};
    ATTACK_INFO m_tAttackInfo{};

    unordered_set<CGameObject*> m_DamagedTargets;

    _bool m_bBurn{};

    _float4x4 m_HammerHeadWorldMatrix{};

public:
    static CKirby_ToyHammer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free();
};

NS_END
