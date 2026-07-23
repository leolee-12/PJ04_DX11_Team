#pragma once

#include "Kirby_OnOffPart.h"

NS_BEGIN(Client)

class CKirby_ToyHammer final : public CKirby_OnOffPart
{
    GENERATED_BODY(CKirby_ToyHammer)

private:
    enum TOY_HAMMER_MESH { HANDLE, HEAD, TOP, MESH_END };

public:
    struct KIRBY_TOYHAMMER_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_ToyHammer";
    static constexpr const wchar_t* Kirby_PartTag = L"ToyHammer";

private:
    CKirby_ToyHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CKirby_ToyHammer(const CKirby_ToyHammer& Prototype);
    virtual ~CKirby_ToyHammer() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    virtual void Set_PartMode(CKirby* pKirby, KIRBY_PART_MODE ePartMode) override;

    void BurnHammer(_bool bBurn) { m_bBurn = bBurn; }

private:
    HRESULT Ready_Components();

private:
    _bool m_bBurn{};

public:
    static CKirby_ToyHammer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free();
};

NS_END
