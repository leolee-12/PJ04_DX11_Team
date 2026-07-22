#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CBoss_Metaknight_EscapeMant final : public CMonsterPart
{
    GENERATED_BODY(CBoss_Metaknight_EscapeMant)

private:
    CBoss_Metaknight_EscapeMant(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Metaknight_EscapeMant(const CBoss_Metaknight_EscapeMant& Prototype);
    virtual ~CBoss_Metaknight_EscapeMant() = default;

public:
    struct BOSS_METAKNIGHT_ESCAPEMANT_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Metaknight_EscapeMant";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Metaknight_EscapeMant";
    static constexpr const wchar_t* PART_TAG = L"EscapeMant";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CBoss_Metaknight_EscapeMant* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Metaknight_EscapeMant* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END