#pragma once

#include "Kirby_OnOffPart.h"

NS_BEGIN(Client)

class CKirby_ToyHat final : public CKirby_OnOffPart
{
    GENERATED_BODY(CKirby_ToyHat)

public:
    struct KIRBY_TOYHAT_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_ToyHat";
    static constexpr const wchar_t* Kirby_PartTag = L"ToyHat";

private:
    CKirby_ToyHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CKirby_ToyHat(const CKirby_ToyHat& Prototype);
    virtual ~CKirby_ToyHat() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CKirby_ToyHat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free();
};

NS_END
