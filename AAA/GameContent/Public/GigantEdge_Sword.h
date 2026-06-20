#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CGigantEdge_Sword final : public CMonsterPart
{
    GENERATED_BODY(CGigantEdge_Sword)

private:
    CGigantEdge_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge_Sword(const CGigantEdge_Sword& Prototype);
    virtual ~CGigantEdge_Sword() = default;

public:
    struct GIGANTEDGE_SWORD_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge_Sword";
    static constexpr const wchar_t* PART_TAG = L"Sword";

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CGigantEdge_Sword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGigantEdge_Sword* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END