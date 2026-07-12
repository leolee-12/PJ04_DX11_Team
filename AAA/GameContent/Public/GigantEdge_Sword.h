#pragma once
#include "MonsterHitPart.h"

NS_BEGIN(Client)

class CGigantEdge_Sword final : public CMonsterHitPart
{
    GENERATED_BODY(CGigantEdge_Sword)

private:
    CGigantEdge_Sword(ID3D11Device*, ID3D11DeviceContext*);
    CGigantEdge_Sword(const CGigantEdge_Sword& Prototype);
    virtual ~CGigantEdge_Sword() = default;

public:
    struct GIGANTEDGE_SWORD_DESC : public CMonsterPart::MONSTERPART_DESC {};
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge_Sword";
    static constexpr const wchar_t* PART_TAG = L"Sword";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CGigantEdge_Sword* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGigantEdge_Sword* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END