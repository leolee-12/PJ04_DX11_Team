#pragma once
#include "MultiHitBoxPart.h"

NS_BEGIN(Client)

class CBoss_Leopard_Body final : public CMultiHitBoxPart
{
    GENERATED_BODY(CBoss_Leopard_Body)

public:
    enum LEOPARD_HITBOX { LHB_BODY, LHB_END };   // TODO: 공격 히트박스 추가

private:
    CBoss_Leopard_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Leopard_Body(const CBoss_Leopard_Body& Prototype);
    virtual ~CBoss_Leopard_Body() = default;

public:
    struct BOSS_LEOPARD_BODY_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Leopard_Body";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Leopard_Body";
    static constexpr const wchar_t* PART_TAG = L"Body";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    enum LEOPARD_PASS : _uint { PASS_SHADOW = 0, PASS_BODY = 1 };
    HRESULT Ready_Components();

public:
    static CBoss_Leopard_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Leopard_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END