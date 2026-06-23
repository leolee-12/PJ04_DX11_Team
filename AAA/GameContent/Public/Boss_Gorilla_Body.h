#pragma once
#include "MultiHitBoxPart.h"

NS_BEGIN(Client)

class CBoss_Gorilla_Body final : public CMultiHitBoxPart
{
    GENERATED_BODY(CBoss_Gorilla_Body)

public:
    enum GORILLA_HITBOX { GHB_RHAND, GHB_LHAND, GHB_END };

private:
    CBoss_Gorilla_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Gorilla_Body(const CBoss_Gorilla_Body& Prototype);
    virtual ~CBoss_Gorilla_Body() = default;

public:
    struct BOSS_GORILLA_BODY_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Gorilla_Body";
    static constexpr const wchar_t* PART_TAG = L"Body";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }
    virtual HRESULT Render() override;

private:
    HRESULT Ready_Components();

public:
    static CBoss_Gorilla_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Gorilla_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END