#pragma once
#include "MonsterHitPart.h"
#include "Inhalable.h"

NS_BEGIN(Client)

class CBoss_Metaknight_Sword final : public CMonsterHitPart, public IInhalable
{
    GENERATED_BODY(CBoss_Metaknight_Sword)

private:
    CBoss_Metaknight_Sword(ID3D11Device*, ID3D11DeviceContext*);
    CBoss_Metaknight_Sword(const CBoss_Metaknight_Sword& Prototype);
    virtual ~CBoss_Metaknight_Sword() = default;

public:
    struct BOSS_METAKNIGHT_SWORD_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight_Sword";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Metaknight_Sword";
    static constexpr const wchar_t* PART_TAG = L"Sword";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    void Drop_Freeze();
    void Retire_Drop();

public:
    _bool Can_BeInhaled(const INHALE_QUERY&) const override { return false; }
    void  Be_Captured(CGameObject*) override {}
    COPY_ABILITY_TYPE Get_CopyAbility() const override { return COPY_ABILITY_TYPE::METAKNIGHT_SWORD; }
    CGameObject* Get_GameObject() override { return this; }
    void On_SpatBegin() override {}
    void On_SpatEnd() override {}

private:
    HRESULT Ready_Components();

private:
    CCollider* m_pPickBox = { nullptr };
    _bool      m_bDropped = { false };
    _bool      m_bTaken = { false };
    _float4x4  m_DropWorld = {};

public:
    static CBoss_Metaknight_Sword* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CBoss_Metaknight_Sword* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END