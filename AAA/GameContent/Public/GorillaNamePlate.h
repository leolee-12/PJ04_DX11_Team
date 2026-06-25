#pragma once
#include "BossNamePlate.h"

NS_BEGIN(Client)

class CGorillaNamePlate final : public CBossNamePlate
{
    GENERATED_BODY(CGorillaNamePlate)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GorillaNamePlate";

private:
    CGorillaNamePlate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGorillaNamePlate(const CGorillaNamePlate& Prototype);
    virtual ~CGorillaNamePlate() = default;

public:
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual const _tchar* Get_ModelProtoTag() const override
    {
        return TEXT("Prototype_Component_Model_GorillaNamePlate");
    }
    // 필요하면 트리거 태그도 변경 가능 (기본 "Boss.NamePlateOn")
    // virtual const _tchar* Get_ActivateEventTag() const override { return TEXT("Boss.Gorilla.NamePlateOn"); }

private:
    static constexpr _float4 s_vColorRed = { 1.f, 0.f, 0.f, 1.f };
    static constexpr _float4 s_vColorWhite = { 1.f, 1.f, 1.f, 1.f };
    static constexpr _float4 s_vColorBlack = { 0.f, 0.f, 0.f, 1.f };

public:
    static CGorillaNamePlate* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END