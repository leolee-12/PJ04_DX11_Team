#pragma once
#include "MonsterHitPart.h"

NS_BEGIN(Client)

class CBoss_Metaknight_ReplicaSword final : public CMonsterHitPart
{
    GENERATED_BODY(CBoss_Metaknight_ReplicaSword)

private:
    CBoss_Metaknight_ReplicaSword(ID3D11Device*, ID3D11DeviceContext*);
    CBoss_Metaknight_ReplicaSword(const CBoss_Metaknight_ReplicaSword& Prototype);
    virtual ~CBoss_Metaknight_ReplicaSword() = default;

public:
    struct BOSS_METAKNIGHT_SWORD_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight_ReplicaSword";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Metaknight_ReplicaSword";
    static constexpr const wchar_t* PART_TAG = L"ReplicaSword";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CBoss_Metaknight_ReplicaSword* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CBoss_Metaknight_ReplicaSword* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END