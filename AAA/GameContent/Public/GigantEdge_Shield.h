#pragma once
#include "MonsterHitPart.h"

NS_BEGIN(Client)

class CGigantEdge_Shield final : public CMonsterHitPart
{
    GENERATED_BODY(CGigantEdge_Shield)

private:
    CGigantEdge_Shield(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge_Shield(const CGigantEdge_Shield& Prototype);
    virtual ~CGigantEdge_Shield() = default;

public:
    struct GIGANTEDGE_SHIELD_DESC : public CMonsterPart::MONSTERPART_DESC {};
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge_Shield";
    static constexpr const wchar_t* PART_TAG = L"Shield";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CGigantEdge_Shield* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGigantEdge_Shield* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END