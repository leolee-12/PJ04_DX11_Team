#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CBoss_Metaknight_Mant final : public CMonsterPart
{
    GENERATED_BODY(CBoss_Metaknight_Mant)

private:
    CBoss_Metaknight_Mant(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Metaknight_Mant(const CBoss_Metaknight_Mant& Prototype);
    virtual ~CBoss_Metaknight_Mant() = default;

public:
    struct BOSS_METAKNIGHT_MANT_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight_Mant";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Metaknight_Mant";
    static constexpr const wchar_t* PART_TAG = L"Mant";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CBoss_Metaknight_Mant* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Metaknight_Mant* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END