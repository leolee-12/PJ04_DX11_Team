#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CGigantEdge_Body final : public CMonsterPart
{
    GENERATED_BODY(CGigantEdge_Body)

private:
    CGigantEdge_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge_Body(const CGigantEdge_Body& Prototype);
    virtual ~CGigantEdge_Body() = default;

public:
    struct GIGANTEDGE_BODY_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge_Body";
    static constexpr const wchar_t* PART_TAG = L"Body";

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    virtual HRESULT Ready_Components() override;

public:
    static CGigantEdge_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGigantEdge_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END